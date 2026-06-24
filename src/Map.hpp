#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Map {
public:
    static const int TILE_SIZE = 16;

    bool load(const std::string& tmxPath);
    void draw(sf::RenderTarget& target);
    bool isSolid(int tileX, int tileY) const;
    bool isCollidingWith(const sf::FloatRect& bounds) const;
    int getCols() const { return m_cols; }
    int getRows() const { return m_rows; }

private:
    struct Tileset {
        int firstgid = 0;
        int cols     = 0;
        sf::Texture  texture;
    };
    using Grid = std::vector<std::vector<uint32_t>>;

    std::vector<Tileset> m_tilesets;
    std::vector<Grid>    m_visuals;
    Grid                 m_collision;
    int m_cols = 0;
    int m_rows = 0;

    bool parseTMX(const std::string& path);
    bool loadTileset(Tileset& ts, const std::string& tsxPath);
    static Grid parseCSV(const std::string& data);
    void drawGrid(sf::RenderTarget& target, const Grid& grid);
    const Tileset* findTileset(int gid) const;
};
