#include <iostream>
#include <fstream>
#include <vector>
#include <string>

class FileReader {
private:
    std::vector<std::string> flights;
public:
    FileReader(const std::string &filePath) {
        std::ifstream configFile(filePath);
        std::string flight;
        if (configFile.is_open()) {
            while (std::getline(configFile, flight)) {
                if (!flight.empty()) {
                    flights.push_back(flight);
                }
            }
            configFile.close();
        } else {
            std::cerr << "Error opening file" << std::endl;
        }
        flights.shrink_to_fit();
    }

    std::vector<std::string> getFlights() const {
        return flights;
    }
};


int main() {
    FileReader fileReader("../config.txt");
    auto flights = fileReader.getFlights();
    for (const auto &flight: flights) {
        std::cout << flight << std::endl;
    }
    return 0;
}
