#include "svg.h"

namespace svg {

    using namespace std::literals;

    bool operator==(const Point& lhs, const Point& rhs) {
        return (lhs.x == rhs.x) && (lhs.y == rhs.y);
    }

    bool operator!=(const Point& lhs, const Point& rhs) {
        return !(lhs.x == rhs.x);
    }

    std::ostream& operator<<(std::ostream& out, Color color) {
        if (std::holds_alternative<std::string>(color)) {
            const std::string& color_ = std::get<std::string>(color);
            return out << color_;
        }
        else if ((std::holds_alternative<Rgb>(color))) {
            const Rgb& color_ = std::get<Rgb>(color);
            return out << "rgb("s << static_cast<int>(color_.red)
                << ","s << static_cast<int>(color_.green)
                << ","s << static_cast<int>(color_.blue)
                << ")"s;
        }
        else if ((std::holds_alternative<Rgba>(color))) {
            const Rgba& color_ = std::get<Rgba>(color);
            return out << "rgba("s << static_cast<int>(color_.red)
                << ","s << static_cast<int>(color_.green)
                << ","s << static_cast<int>(color_.blue)
                << ","s << color_.opacity
                << ")"s;
        }
        else {
            return out << "none"s;
        }
    }

     

    void ColorDetection::operator()(std::monostate) const {
        out << "none";
    }

    void ColorDetection::operator()(std::string color) const {
        out << color;
    }

    void ColorDetection::operator()(Rgb color) const {
        out << "rgb("s << color.red
            << ","s << color.green
            << ","s << color.blue
            << ")"s;

    }

    void ColorDetection::operator()(Rgba color) const {
        out << "rgba("s << color.red
            << ","s << color.green
            << ","s << color.blue
            << ","s << color.opacity
            << ")"s;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap elem) {
        switch (elem) {
        case StrokeLineCap::BUTT: return out << "butt"s;
        case StrokeLineCap::ROUND: return out << "round"s;
        case StrokeLineCap::SQUARE: return out << "square"s;
        default: return out << "no such LineCap"s;
        }
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin elem) {
        switch (elem) {
        case StrokeLineJoin::ARCS: return out << "arcs"s;
        case StrokeLineJoin::BEVEL: return out << "bevel"s;
        case StrokeLineJoin::MITER: return out << "miter"s;
        case StrokeLineJoin::MITER_CLIP: return out << "miter-clip"s;
        case StrokeLineJoin::ROUND: return out << "round"s;
        default: return out << "no such LineJoin"s;
        }
    }

    // ---------- Circle ------------------
    Circle& Circle::SetCenter(Point center) {
        center_ = std::move(center);
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = std::move(radius);
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------
    Polyline& Polyline::AddPoint(Point point) {
        polyline_.push_back(std::move(point));
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool IsFirst = true;
        for (const auto& point : polyline_) {
            if (IsFirst) {
                IsFirst = false;
            }
            else {
                out << " "sv;
            }
            out << point.x;
            out << ","sv;
            out << point.y;
        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;


    }

    // ---------- Text ------------------
    Text& Text::SetPosition(Point pos) {
        pos_ = std::move(pos);
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = std::move(offset);
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }


    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }


    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        
        out << "<text "sv;
        RenderAttrs(context.out);
        out <<" x=\""s << pos_.x << "\" y=\""sv << pos_.y << "\" dx=\""sv;
        out << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv << size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        
        out << ">"sv;
        if (!data_.empty()) {
            out << data_;
        }
        out << "</text>"sv;
    }


    // ---------- Document ------------------
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(move(obj));
    }


    void Document::Render(std::ostream& out) const {
        auto svg = RenderContext(out, 2, 2);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
        for (size_t i = 0; i < objects_.size(); ++i) {
            objects_[i]->Render(svg);
        }
        out << "</svg>"sv;
    }
}  // namespace svg