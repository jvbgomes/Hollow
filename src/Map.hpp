#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Map {
public:
    static const int TILE_SIZE = 32;

    bool load(const std::string& csvPath, const std::string& tilesetPath);
    void draw(sf::RenderWindow& window);

    bool isSolid(int tileX, int tileY) const;
    bool isCollidingWith(const sf::FloatRect& bounds) const;

    int getCols() const;
    int getRows() const;

private:
    std::vector<std::vector<int>> grid;
    sf::Texture tilesetTexture;
    sf::Sprite tileSprite;
    int tilesetCols = 0;
};
