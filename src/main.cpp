#include <string>
#include <iostream>

#include <boost/program_options.hpp>

using std::cout;
using std::string;
using std::endl;

namespace po = boost::program_options;

int main(int argc, char ** argv)
{
    int start = 0;
    int end = 0;
    string url;
    string download_location;
    bool download = false;

    po::options_description desc("Options");
    desc.add_options()
        ("help", "produce help message")
        ("start", po::value<int>(), "Start number")
        ("end", po::value<int>(), "End number")
        ("download", po::value<bool>()->default_value(false), "Should download files or not")
        ("url", po::value<string>(), "Url of The page to try every possible file between start and end in the form of C-style printf")
        ("download_location", po::value<string>(), "Path to download location")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    try {
        po::notify(vm);
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    if (vm.count("help")) {
        cout << desc << endl;
        return 1;
    }

    if (vm.count("start")) {
        cout << "Start number in url" 
        << vm["start"].as<int>() << endl;
        start = vm["start"].as<int>();
    } else {
        cout << "start number should be provided." << endl;
        return 1;
    }

    if (vm.count("end")) {
        cout << "end number in url" 
        << vm["end"].as<int>() << endl;
        end = vm["end"].as<int>();
    } else {
        cout << "end number should be provided." << endl;
        return 1;
    }

    if(vm.count("download")) {
        download = true;
        if(vm.count("download_location")) {
            cout << "Download location is : " << download_location << endl;
            download_location = vm["download_location"].as<string>();
        } else {
            cout << "download location should be provided." << endl;
            return 1;
        }
    } else {
        if(vm.count("download_location")) {
            cout << "download flag should provided before using download_location." << endl;
            return 1;
        } 
    }


    return 0;
}
