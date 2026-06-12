#include "Map.hpp"
#include <fstream>
#include <sstream>

bool Map::load(const std::string& csvPath, const std::string& tilesetPath) {
    if (!tilesetTexture.loadFromFile(tilesetPath))
        return false;

    tilesetCols = tilesetTexture.getSize().x / TILE_SIZE;
    tileSprite.setTexture(tilesetTexture);

    std::ifstream file(csvPath);
    if (!file.is_open())
        return false;

    grid.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::vector<int> row;
        std::stringstream ss(line);
        std::string cell;
        while (std::getline(ss, cell, ',')) {
            if (!cell.empty())
                row.push_back(std::stoi(cell));
        }
        if (!row.empty())
            grid.push_back(row);
    }

    return !grid.empty();
}

void Map::draw(sf::RenderWindow& window) {
    for (int y = 0; y < (int)grid.size(); y++) {
        for (int x = 0; x < (int)grid[y].size(); x++) {
            int id = grid[y][x];
            if (id <= 0) continue;

            // Calcula qual tile recortar do tileset
            int col = (id - 1) % tilesetCols;
            int row = (id - 1) / tilesetCols;

            tileSprite.setTextureRect(sf::IntRect(
                col * TILE_SIZE,
                row * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE
            ));
            tileSprite.setPosition((float)(x * TILE_SIZE), (float)(y * TILE_SIZE));
            window.draw(tileSprite);
        }
    }
}

bool Map::isSolid(int tileX, int tileY) const {
    if (tileY < 0 || tileY >= (int)grid.size())    return true;
    if (tileX < 0 || tileX >= (int)grid[tileY].size()) return true;
    return grid[tileY][tileX] != 0;
}

bool Map::isCollidingWith(const sf::FloatRect& bounds) const {
    int left   = (int)bounds.left / TILE_SIZE;
    int right  = (int)(bounds.left + bounds.width  - 1) / TILE_SIZE;
    int top    = (int)bounds.top  / TILE_SIZE;
    int bottom = (int)(bounds.top  + bounds.height - 1) / TILE_SIZE;

    for (int y = top; y <= bottom; y++)
        for (int x = left; x <= right; x++)
            if (isSolid(x, y)) return true;

    return false;
}

int Map::getCols() const { return grid.empty() ? 0 : (int)grid[0].size(); }
int Map::getRows() const { return (int)grid.size(); }
