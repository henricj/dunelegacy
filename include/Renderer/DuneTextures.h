#ifndef DUNETEXTURES_H
#define DUNETEXTURES_H

#include <gsl/gsl>

#include <FileClasses/GFXConstants.h>

#include "DuneTexture.h"

class SurfaceLoader;

class DuneTextures final {
public:
    ~DuneTextures();

    static DuneTextures create(SDL_Renderer* renderer, SurfaceLoader* manager);

    const DuneTexture& get_object_picture(unsigned int id, HOUSETYPE house, int zoom) const {
        return object_pictures_.at(zoom).at(id).at(static_cast<int>(house));
    }
    const DuneTexture& get_small_object(unsigned int id) const { return small_details_.at(id); }
    const DuneTexture& get_tiny_picture(unsigned int id) const { return tiny_pictures_.at(id); }
    const DuneTexture& get_ui_graphic(unsigned int id, HOUSETYPE house) const {
        return ui_graphics_.at(static_cast<int>(house)).at(id);
    }
    const DuneTexture& get_generated_picture(GeneratedPicture id) const { return generated_pictures_.at(static_cast<int>(id)); }

    using object_pictures_type =
        std::array<std::array<std::array<DuneTexture, static_cast<int>(HOUSETYPE::NUM_HOUSES)>, NUM_OBJPICS>,
                   NUM_ZOOMLEVEL>;
    using small_details_type = std::array<DuneTexture, NUM_SMALLDETAILPICS>;
    using tiny_pictures_type = std::array<DuneTexture, NUM_TINYPICTURE>;
    using ui_graphics_type = std::array<std::array<DuneTexture, NUM_UIGRAPHICS>, static_cast<int>(HOUSETYPE::NUM_HOUSES)>;
    using generated_type = std::array<DuneTexture, NUM_GENERATEDPICTURES>;

private:
    DuneTextures();
    DuneTextures(std::vector<sdl2::texture_ptr>&& textures, object_pictures_type&& object_pictures,
                 small_details_type&& small_details, tiny_pictures_type&& tiny_pictures, ui_graphics_type&& ui_graphics,
                 generated_type&& generated_pictures);

    const object_pictures_type object_pictures_{};
    const small_details_type   small_details_{};
    const tiny_pictures_type   tiny_pictures_{};
    const ui_graphics_type     ui_graphics_{};
    const generated_type       generated_pictures_{};

    std::vector<sdl2::texture_ptr> textures_;
};

#endif // DUNETEXTURES_H
