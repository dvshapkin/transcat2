#pragma once

#include "transport_catalogue.h"

namespace transcat::query {

    // Обрабатывает информационные запросы
    void ReadQuery(TransportCatalogue &catalogue, std::istream &in, std::ostream &out);

    namespace out {

        // Вывод информации о маршруте
        void PrintBusInfo(std::ostream &out, const Bus *p, const TransportCatalogue &catalogue);

        // Вывод всех автобусов, проходящих через остановку
        void PrintBusesForStop(std::ostream &out, StopPtr p_stop, const std::set<const Bus *, TransportCatalogue::BusPtrComparator> * buses);

    } //namespace transcat::query::out

    namespace detail {

        // Читает строку из потока ввода
        std::string ReadLine(std::istream &in);

        // Читает число из потока ввода
        int ReadLineWithNumber(std::istream &in);

        // Удаляет лидирующие и концевые пробелы в строке
        std::string_view Trim(std::string_view sv);

        // Делит строку на две части по первому символу-разделителю
        std::pair<std::string_view, std::string_view>
        SplitOn(std::string_view sv, char delimiter = ' ', size_t offset = 0);

    } //namespace transcat::query::detail

} //namespace transcat::query