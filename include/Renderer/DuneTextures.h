#ifndef DUNETEXTURES_H
#define DUNETEXTURES_H

#include <FileClasses/GFXConstants.h>

#include "DataTypes.h"
#include "DuneTexture.h"

class SurfaceLoader;

class DuneTextures final {
public:
    ~DuneTextures();

    static DuneTextures create(SDL_Renderer* renderer, SurfaceLoader* manager);

    [[nodiscard]] const DuneTexture& get_object_picture(unsigned int id, HOUSETYPE house, int zoom) const {
        return object_pictures_.at(zoom).at(id).at(static_cast<int>(house));
    }

    [[nodiscard]] const DuneTexture& get_small_object(unsigned int id) const { return small_details_.at(id); }
    [[nodiscard]] const DuneTexture& get_tiny_picture(unsigned int id) const { return tiny_pictures_.at(id); }

    [[nodiscard]] const DuneTexture& get_ui_graphic(UIGraphics_Enum id, HOUSETYPE house) const {
        return ui_graphics_.at(static_cast<int>(house)).at(id);
    }

    [[nodiscard]] const DuneTexture& get_map_choice(unsigned int num, HOUSETYPE house) const {
        return map_choice_.at(static_cast<int>(house)).at(num);
    }

    [[nodiscard]] const DuneTexture& get_generated_picture(GeneratedPicture id) const {
        return generated_pictures_.at(static_cast<int>(id));
    }

    struct DecorationBorderType {
        DuneTexture ball;
        DuneTexture hspacer;
        DuneTexture vspacer;
        DuneTexture hborder;
        DuneTexture vborder;
    };

    struct BorderStyle {
        DuneTexture leftUpperCorner;
        DuneTexture rightUpperCorner;
        DuneTexture leftLowerCorner;
        DuneTexture rightLowerCorner;
        std::vector<SDL_Color> hborder;
        std::vector<SDL_Color> vborder;
    };

    [[nodiscard]] DecorationBorderType getDecorationBorder() const { return decoration_border_; }
    [[nodiscard]] BorderStyle getBorderStyle(DecorationFrame type) const {
        return border_style_.at(static_cast<int>(type));
    }

    using object_pictures_type =
        std::array<std::array<std::array<DuneTexture, static_cast<int>(HOUSETYPE::NUM_HOUSES)>, NUM_OBJPICS>,
                   NUM_ZOOMLEVEL>;
    using small_details_type = std::array<DuneTexture, NUM_SMALLDETAILPICS>;
    using tiny_pictures_type = std::array<DuneTexture, NUM_TINYPICTURE>;
    using ui_graphics_type =
        std::array<std::array<DuneTexture, NUM_UIGRAPHICS>, static_cast<int>(HOUSETYPE::NUM_HOUSES)>;
    using map_choice_type =
        std::array<std::array<DuneTexture, NUM_MAPCHOICEPIECES>, static_cast<int>(HOUSETYPE::NUM_HOUSES)>;
    using generated_type         = std::array<DuneTexture, NUM_GENERATEDPICTURES>;
    using decoration_border_type = DecorationBorderType;
    using border_style_type      = std::array<BorderStyle, NUM_DECORATIONFRAMES>;

private:
    DuneTextures();
    DuneTextures(std::vector<sdl2::texture_ptr>&& textures, object_pictures_type&& object_pictures,
                 small_details_type&& small_details, tiny_pictures_type&& tiny_pictures, ui_graphics_type&& ui_graphics,
                 map_choice_type&& map_choice_, generated_type&& generated_pictures,
                 decoration_border_type&& decoration_border, border_style_type&& border_style);

    const object_pictures_type object_pictures_{};
    const small_details_type small_details_{};
    const tiny_pictures_type tiny_pictures_{};
    const ui_graphics_type ui_graphics_{};
    const map_choice_type map_choice_{};
    const generated_type generated_pictures_{};
    const decoration_border_type decoration_border_{};
    const border_style_type border_style_{};

    std::vector<sdl2::texture_ptr> textures_;
};

#endif // DUNETEXTURES_H
