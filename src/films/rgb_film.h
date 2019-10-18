//
// Created by Mike Smith on 2019/10/18.
//

#pragma once

#include "core/film.h"

namespace luisa {

DERIVED_CLASS(RGBFilm, Film) {
    
    CREATOR("RGB") {
        auto film = std::make_shared<RGBFilm>();
        if (!film->_decode_size(param_set)) {
            std::cerr << "Film size not defined, using default: 1280x720." << std::endl;
            film->_size = glm::uvec2{1280, 720};
        }
        if (!film->_decode_filter(param_set)) {
            std::cerr << "Filter not defined, using default: Neareast." << std::endl;
        }
        return film;
    }
    
};

}


