#include <iostream>
#include <iomanip>
#include <iterator>
#include <optional>

#include "transport_router.h"
#include "transport_catalogue.h"

namespace transcat::query {

    namespace detail {

        // Читает строку из потока ввода
        std::string ReadLine(std::istream &in) {
            std::string s;
            getline(in, s);
            return s;
        }

        // Читает число из потока ввода
        int ReadLineWithNumber(std::istream &in) {
            int result;
            in >> result;
            ReadLine(in);
            return result;
        }

        // Делит строку на две части по первому символу-разделителю
        std::pair<std::string_view, std::string_view> SplitOn(std::string_view sv, char delimiter, size_t offset) {
            size_t delim_pos = sv.find_first_of(delimiter, offset);
            if (delim_pos != std::string_view::npos) {
                if (delim_pos + 1 < sv.size()) {
                    return {{&sv[0],             delim_pos},
                            {&sv[delim_pos + 1], sv.size() - delim_pos - 1}};
                }
                return {{&sv[0], delim_pos},
                        {}};
            }
            return {{&sv[0]},
                    {}};
        }
    }

    // Обрабатывает информационные запросы
    void ReadQuery(TransportCatalogue &catalogue, std::istream &in, std::ostream &out) {
        size_t query_count = detail::ReadLineWithNumber(in);
        for (size_t i = 0; i < query_count; ++i) {
            std::string query = detail::ReadLine(in);
            auto parts = detail::SplitOn(query);

            if (parts.first == "Bus") {
                const Bus *p_bus = catalogue.GetBus(parts.second);
                if (p_bus) {
                    out::PrintBusInfo(out, p_bus, catalogue);
                } else {
                    out << "Bus " << parts.second << ": not found" << std::endl;
                }
            }

            if (parts.first == "Stop") {
                StopPtr p_stop = catalogue.GetStop(parts.second);
                if (p_stop) {
                    out::PrintBusesForStop(out, p_stop, catalogue.GetBusesForStop(p_stop));
                } else {
                    out << "Stop " << parts.second << ": not found" << std::endl;
                }
            }
        }
    }


    // Вывод информации о маршруте
    void out::PrintBusInfo(std::ostream &out, const Bus *p, const TransportCatalogue &catalogue) {
        distance_t length = geo::ComputeRouteLength(p, catalogue);
        double geo_length = geo::ComputeRouteGeoLength(p);

        out << "Bus " << p->name << ": "
            << p->route.size() << " stops on route, "
            << p->unique_stops << " unique stops, "
            << length << " route length, "
            << std::setprecision(6)
            << length / geo_length << " curvature"
            << std::endl;
    }

    // Вывод всех автобусов, проходящих через остановку
    void out::PrintBusesForStop(std::ostream &out, StopPtr p_stop, const std::set<const Bus *, TransportCatalogue::BusPtrComparator> * buses) {
        out << "Stop " << p_stop->name << ": ";
        if (buses->empty()) {
            out << "no buses";
        } else {
            out << "buses";
            for (const auto p_bus: *buses) {
                out << ' ' << p_bus->name;
            }
        }
        out << std::endl;
    }

} //namespace transcat::query
