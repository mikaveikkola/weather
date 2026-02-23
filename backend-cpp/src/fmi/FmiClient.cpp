#include "FmiClient.h"
#include "StoredQueries.h"

#include <curl/curl.h>
#include <sstream>

namespace weather::fmi {

static size_t writeCallback(void* data, size_t size, size_t nmemb, std::string* out) {
    out->append(static_cast<char*>(data), size * nmemb);
    return size * nmemb;
}

FetchResult FmiClient::doGet(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) return {false, "", "curl_easy_init failed"};

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL,           url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,       60L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,1L);

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        return {false, "", std::string(curl_easy_strerror(res))};
    if (httpCode >= 400)
        return {false, "", "HTTP " + std::to_string(httpCode)};
    return {true, std::move(response), ""};
}

FetchResult FmiClient::fetchObservations(const std::string& starttime,
                                          const std::string& endtime,
                                          const std::string& bbox)
{
    std::ostringstream url;
    url << FMI_WFS_BASE
        << "?service=WFS&version=2.0.0&request=getFeature"
        << "&storedquery_id=" << OBSERVATIONS_MULTIPOINTCOVERAGE
        << "&starttime=" << starttime
        << "&endtime="   << endtime
        << "&bbox="      << bbox
        << "&timestep=10";
    return doGet(url.str());
}

FetchResult FmiClient::fetchForecastHarmonie(const std::string& place) {
    std::ostringstream url;
    url << FMI_WFS_BASE
        << "?service=WFS&version=2.0.0&request=getFeature"
        << "&storedquery_id=" << FORECAST_HARMONIE
        << "&place=" << place;
    return doGet(url.str());
}

} // namespace weather::fmi
