#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg {

    struct Rgb {
    public:
        Rgb(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0)
                : red(r), green(g), blue(b) {
        }

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    std::ostream& operator<<(std::ostream& out, Rgb color);

    struct Rgba : public Rgb {
    public:
        Rgba(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, double a = 1.0)
                : Rgb(r, g, b), opacity(a) {
        }

        double opacity = 1.0;
    };

    std::ostream& operator<<(std::ostream& out, Rgba color);

    using Color = std::variant<std::string, Rgb, Rgba>;

    struct OstreamColorPrinter {
        std::ostream& out;

        void operator()(const std::string& color) const {
            out << color;
        }
        void operator()(Rgb color) const {
            out << color;
        }
        void operator()(Rgba color) const {
            out << color;
        }
    };

    // Объявив в заголовочном файле константу со спецификатором inline,
    // мы сделаем так, что она будет одной на все единицы трансляции,
    // которые подключают этот заголовок.
    // В противном случае каждая единица трансляции будет использовать свою копию этой константы
    inline const Color NoneColor{"none"};

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    std::ostream& operator<<(std::ostream& out, StrokeLineCap linecap);

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin linejoin);

    template<typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv; //<< *fill_color_
                std::visit(OstreamColorPrinter{out}, *fill_color_);
                out << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv; //<< *stroke_color_
                std::visit(OstreamColorPrinter{out}, *stroke_color_);
                out << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_line_cap_) {
                out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
            }
            if (stroke_line_join_) {
                out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;

    };

    class Object;

    class ObjectContainer {
    public:
        virtual void AddPtr(std::unique_ptr<Object> &&) = 0;

        template<typename T>
        void Add(T obj) {
            AddPtr(std::make_unique<T>(obj));
        }
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer &container) const = 0;

        virtual ~Drawable() = default;
    };

    struct Point {
        Point() = default;

        Point(double x, double y)
                : x(x), y(y) {
        }

        double x = 0;
        double y = 0;
    };

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
    struct RenderContext {
        RenderContext(std::ostream &out)
                : out(out) {
        }

        RenderContext(std::ostream &out, int indent_step, int indent = 0)
                : out(out), indent_step(indent_step), indent(indent) {
        }

        RenderContext Indented() const {
            return {out, indent_step, indent + indent_step};
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream &out;
        int indent_step = 0;
        int indent = 0;
    };

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
    class Object {
    public:
        void Render(const RenderContext &context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext &context) const = 0;
    };

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle &SetCenter(Point center);

        Circle &SetRadius(double radius);

    private:
        void RenderObject(const RenderContext &context) const override;

        Point center_;
        double radius_ = 1.0;
    };

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline &AddPoint(Point point);

    private:
        void RenderObject(const RenderContext &context) const override;

        std::vector<Point> points_;
    };

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
    class Text final : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text &SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text &SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text &SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text &SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text &SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text &SetData(std::string data);

    private:
        void RenderObject(const RenderContext &context) const override;

        Point position_;
        Point offset_;
        uint32_t font_size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };

    class Document : public ObjectContainer {
    public:
        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object> &&obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream &out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

}  // namespace svg
