//
// Created by Mike Smith on 2019/10/18.
//

#pragma once

#include "core/film.h"

namespace luisa {

DERIVED_CLASS(RGBFilm, Film) {
    
    CREATOR("RGB") {
        auto film = std::make_shared<RGBFilm>();
        film->_decode(param_set);
        return film;
    }
    
};

}
