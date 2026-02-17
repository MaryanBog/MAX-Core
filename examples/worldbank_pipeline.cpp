#include <maxcore/maxcore.h>

#include <curl/curl.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace maxcore;

// =====================================================
// CURL
// =====================================================

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    const size_t total = size * nmemb;
    std::string* s = static_cast<std::string*>(userp);
    s->append(static_cast<char*>(contents), total);
    return total;
}

static std::string http_get(const std::string& url)
{
    CURL* curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("curl init failed");

    std::string buffer;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    const CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        throw std::runtime_error("curl request failed");

    return buffer;
}

// =====================================================
// Minimal JSON extraction (WorldBank format)
// =====================================================

static std::map<int, double> parse_series(const std::string& json)
{
    std::map<int, double> series;
    size_t pos = 0;

    while (true) {
        pos = json.find("\"date\"", pos);
        if (pos == std::string::npos)
            break;

        const size_t y1 = json.find("\"", pos + 6);
        const size_t y2 = json.find("\"", y1 + 1);
        const int year = std::stoi(json.substr(y1 + 1, y2 - y1 - 1));

        const size_t value_pos = json.find("\"value\"", y2);
        const size_t colon = json.find(":", value_pos);
        const size_t comma = json.find(",", colon);

        const std::string val_str = json.substr(colon + 1, comma - colon - 1);

        try {
            const double value = std::stod(val_str);
            series[year] = value;
        } catch (...) {
            // null -> ignore
        }

        pos = comma;
    }

    return series;
}

// =====================================================
// Z-score normalization
// =====================================================

static void normalize(std::map<int, double>& data)
{
    if (data.empty())
        return;

    const double n = static_cast<double>(data.size());

    double mean = 0.0;
    for (const auto& kv : data)
        mean += kv.second;
    mean /= n;

    double var = 0.0;
    for (const auto& kv : data) {
        const double d = kv.second - mean;
        var += d * d;
    }
    var /= n;

    double stddev = std::sqrt(var);
    if (std::abs(stddev) < 1e-12)
        stddev = 1.0;

    for (auto& kv : data)
        kv.second = (kv.second - mean) / stddev;
}

// =====================================================
// Data pack per country (download once, reuse for scans)
// =====================================================

struct CountryData {
    std::string code;
    std::vector<int> years;                         // ascending
    std::vector<std::array<double, 3>> deltas_norm; // per year
};

static CountryData load_country(const std::string& country_code)
{
    const std::string base =
        "https://api.worldbank.org/v2/country/" + country_code + "/indicator/";

    const std::string gdp_url   = base + "NY.GDP.MKTP.KD.ZG?format=json&per_page=200";
    const std::string inf_url   = base + "FP.CPI.TOTL.ZG?format=json&per_page=200";
    const std::string unemp_url = base + "SL.UEM.TOTL.ZS?format=json&per_page=200";

    auto gdp   = parse_series(http_get(gdp_url));
    auto inf   = parse_series(http_get(inf_url));
    auto unemp = parse_series(http_get(unemp_url));

    normalize(gdp);
    normalize(inf);
    normalize(unemp);

    std::vector<int> years;
    years.reserve(gdp.size());
    for (const auto& kv : gdp) {
        const int y = kv.first;
        if (inf.count(y) && unemp.count(y))
            years.push_back(y);
    }
    std::sort(years.begin(), years.end());

    CountryData out;
    out.code = country_code;
    out.years = years;
    out.deltas_norm.reserve(years.size());

    for (int y : years) {
        out.deltas_norm.emplace_back(std::array<double, 3>{ gdp[y], inf[y], unemp[y] });
    }

    return out;
}

// =====================================================
// Run one scenario on preloaded data
// =====================================================

static bool run_scenario(
    const CountryData& cd,
    double rho,
    double lambda_phi,
    double lambda_m_fixed,
    int& collapse_year_out
)
{
    ParameterSet p;
    p.alpha = 0.1;
    p.eta = 0.2;
    p.beta = 0.1;
    p.gamma = 0.1;
    p.rho = rho;
    p.lambda_phi = lambda_phi;
    p.lambda_m = lambda_m_fixed;
    p.kappa_max = 1.0;

    StructuralState init{0.0, 0.0, 1.0};

    auto core_opt = MaxCore::Create(p, 3, init);
    if (!core_opt)
        throw std::runtime_error("MaxCore::Create failed");

    MaxCore core = *core_opt;

    for (size_t i = 0; i < cd.years.size(); ++i) {
        const auto& dv = cd.deltas_norm[i];
        const double delta[3] = { dv[0], dv[1], dv[2] };

        const EventFlag ev = core.Step(delta, 3, 1.0);
        if (ev == EventFlag::COLLAPSE) {
            collapse_year_out = cd.years[i];
            return true;
        }
    }

    collapse_year_out = -1;
    return false;
}

// =====================================================
// Main
// =====================================================

int main()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    try {
        const double lambda_m_fixed = 0.05;

        const double rhos[]  = {0.05, 0.15, 0.30};
        const double lphis[] = {0.02, 0.05, 0.10, 0.20};

        const CountryData usa = load_country("USA");
        const CountryData euu = load_country("EUU");

        std::cout << "country,rho,lambda_phi,lambda_m,collapse_year\n";

        for (double rho : rhos) {
            for (double lp : lphis) {

                int cy = -1;
                const bool c1 = run_scenario(usa, rho, lp, lambda_m_fixed, cy);
                std::cout << "USA," << rho << "," << lp << "," << lambda_m_fixed << ","
                          << (c1 ? std::to_string(cy) : "NONE") << "\n";

                cy = -1;
                const bool c2 = run_scenario(euu, rho, lp, lambda_m_fixed, cy);
                std::cout << "EUU," << rho << "," << lp << "," << lambda_m_fixed << ","
                          << (c2 ? std::to_string(cy) : "NONE") << "\n";
            }
        }
    }
    catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << "\n";
    }

    curl_global_cleanup();
    return 0;
}
