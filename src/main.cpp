#include <string>
#include <iostream>
#include <fstream>

using std::string;

#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/exception/exception.hpp>
#include <boost/exception/diagnostic_information.hpp> 
#include <boost/exception_ptr.hpp> 

namespace po = boost::program_options;
using boost::format;
using boost::exception;

#include <curl/curl.h>

#define ERR 0xFFFFFFF1

int main(int argc, char ** argv)
{
    long long start = 0;
    long long end = 0;
    string url;
    string tor;
    string folder_name;
    bool download = false;

    std::ofstream allFiles;
    allFiles.open("allFiles.txt");
    std::ofstream existingFiles;
    existingFiles.open("existingFiles.txt");

    po::options_description desc("Options");
    desc.add_options()
        ("help", "produce help message")
        ("start", po::value<long long>(), "Start number")
        ("end", po::value<long long>(), "End number")
        ("download", po::value<bool>(), "Should download files or not")
        ("url", po::value<string>(), "Url of The page to try every possible file between start and end in the form of C-style prlong longf")
        ("folder_name", po::value<string>(), "folder name to download")
        ("tor", po::value<string>(), "tor ip port like 127.0.0.1:9999")
    ;

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (exception& e) {
        BOOST_LOG_TRIVIAL(fatal) << "Provided arguments are wrong ";
        BOOST_LOG_TRIVIAL(fatal) << "Error: " << boost::diagnostic_information(e);
        return 1;
    }

    if (vm.count("help")) {
        BOOST_LOG_TRIVIAL(error) << desc;
        return 1;
    }

    if (vm.count("url")) {
        BOOST_LOG_TRIVIAL(info) << "url is : " 
        << vm["url"].as<string>();
        url = vm["url"].as<string>();
    } else {
        BOOST_LOG_TRIVIAL(error) << "url should be provided.";
        return 1;
    }

    if (vm.count("tor")) {
        BOOST_LOG_TRIVIAL(info) << "tor address is : " 
        << vm["tor"].as<string>();
        tor = vm["tor"].as<string>();
    } else {
        BOOST_LOG_TRIVIAL(error) << "tor address should be provided.";
        return 1;
    }

    if (vm.count("start")) {
        BOOST_LOG_TRIVIAL(info) << "Start number : " 
        << vm["start"].as<long long>();
        start = vm["start"].as<long long>();
    } else {
        BOOST_LOG_TRIVIAL(error) << "start number should be provided.";
        return 1;
    }

    if (vm.count("end")) {
        BOOST_LOG_TRIVIAL(info) << "end number : " 
        << vm["end"].as<long long>();
        end = vm["end"].as<long long>();

        if(end <= start){
            BOOST_LOG_TRIVIAL(error) << "start number should be bigger than end number.";
            return 1;
        }
    } else {
        BOOST_LOG_TRIVIAL(error) << "end number should be provided.";
        return 1;
    }

    if(vm.count("download")) {
        download = true;
        if(vm.count("folder_name")) {
            folder_name = vm["folder_name"].as<string>();
            BOOST_LOG_TRIVIAL(info) << "folder name is : " << folder_name;
        } else {
            BOOST_LOG_TRIVIAL(error) << "folder name should be provided.";
            return 1;
        }
    } else {
        if(vm.count("folder_name")) {
            BOOST_LOG_TRIVIAL(error) << "download flag should provided before using folder_name.";
            return 1;
        } 
    }


    CURL *curl; 
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT); 
    curl = curl_easy_init(); 
        if(!curl) {
        BOOST_LOG_TRIVIAL(fatal) << "couldn't initilize curl";
        return 1;
    }
  
    // Checking
    for(long long i = start; i < end; ++i) {
        try{
            string t = (format(url.c_str()) % i).str();
            curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5); 
            curl_easy_setopt(curl, CURLOPT_PROXY, tor.c_str());
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
            curl_easy_setopt(curl, CURLOPT_URL, t.c_str());
            res = curl_easy_perform(curl);
            if(res == CURLE_OK) {
                long response_code;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
                BOOST_LOG_TRIVIAL(info) << t  << " " << response_code;
                allFiles << t << " -- " << response_code << "\n";
                if(response_code <= 400){
                    existingFiles << t << "\n";
                }
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "Couldn't execute curl_easy_perform, error code : " << res << " : " << curl_easy_strerror(res); 
            }
        } catch(exception& e){
            BOOST_LOG_TRIVIAL(fatal) << "Wrong format " << boost::diagnostic_information(e);
        }
    }

    // Downloading
    if(download) {
        
    }

    return 0;
}
