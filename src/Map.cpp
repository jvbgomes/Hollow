#include "Map.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <iostream>

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    return { std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>() };
}

static std::string dirOf(const std::string& path) {
    auto p = path.find_last_of("/\\");
    return (p == std::string::npos) ? "./" : path.substr(0, p + 1);
}

static std::string attr(const std::string& src, const std::string& key) {
    std::string k = key + "=\"";
    auto p = src.find(k);
    if (p == std::string::npos) return {};
    p += k.size();
    auto e = src.find('"', p);
    return (e == std::string::npos) ? std::string{} : src.substr(p, e - p);
}

Map::Grid Map::parseCSV(const std::string& data) {
    Grid grid;
    std::istringstream ss(data);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        std::vector<uint32_t> row;
        std::istringstream ls(line);
        std::string cell;
        while (std::getline(ls, cell, ',')) {
            auto first = cell.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) continue;
            auto last = cell.find_last_not_of(" \t\r\n");
            long long v = std::stoll(cell.substr(first, last - first + 1));
            row.push_back((uint32_t)(v < 0 ? 0 : v));
        }
        if (!row.empty()) grid.push_back(row);
    }
    return grid;
}

bool Map::loadTileset(Tileset& ts, const std::string& tsxPath) {
    std::string content = readFile(tsxPath);
    if (content.empty()) return false;

    auto p = content.find("<tileset ");
    ts.cols = std::stoi(attr(content.substr(p), "columns"));

    auto imgPos = content.find("<image ");
    if (imgPos == std::string::npos) return false;

    std::string imgSrc = attr(content.substr(imgPos), "source");
    return ts.texture.loadFromFile(dirOf(tsxPath) + imgSrc);
}

bool Map::parseTMX(const std::string& path) {
    m_tilesets.clear();
    m_visuals.clear();
    m_collision.clear();

    std::string content = readFile(path);
    if (content.empty()) return false;

    std::string base = dirOf(path);

    auto mp = content.find("<map ");
    if (mp == std::string::npos) return false;
    std::string mapTag = content.substr(mp, content.find('>', mp) - mp);
    m_cols = std::stoi(attr(mapTag, "width"));
    m_rows = std::stoi(attr(mapTag, "height"));

    std::size_t pos = 0;
    while ((pos = content.find("<tileset ", pos)) != std::string::npos) {
        auto end = content.find("/>", pos);
        std::string tag = content.substr(pos, end - pos);
        Tileset ts;
        ts.firstgid = std::stoi(attr(tag, "firstgid"));
        std::string tsxPath = base + attr(tag, "source");
        if (!loadTileset(ts, tsxPath)) {
            std::cerr << "Map: falhou ao carregar tileset: " << tsxPath << "\n";
            pos = end;
            continue;
        }
        m_tilesets.push_back(std::move(ts));
        pos = end;
    }
    std::sort(m_tilesets.begin(), m_tilesets.end(),
              [](const Tileset& a, const Tileset& b){ return a.firstgid > b.firstgid; });

    static const std::vector<std::string> VISUAL_ORDER = {
        "Floor", "Floor2", "UnderWall", "Walls", "Painting", "Stairs",
        "Furniture", "Furniture2", "Banister", "Props", "Props2"
    };
    std::map<std::string, Grid> layerMap;

    pos = 0;
    while ((pos = content.find("<layer ", pos)) != std::string::npos) {
        auto tagEnd   = content.find('>', pos);
        std::string name = attr(content.substr(pos, tagEnd - pos), "name");
        auto dataStart = content.find("<data", tagEnd);
        auto csvStart  = content.find('>', dataStart) + 1;
        auto csvEnd    = content.find("</data>", csvStart);
        layerMap[name] = parseCSV(content.substr(csvStart, csvEnd - csvStart));
        pos = csvEnd;
    }

    for (const auto& name : VISUAL_ORDER) {
        auto it = layerMap.find(name);
        if (it != layerMap.end())
            m_visuals.push_back(std::move(it->second));
    }

    auto it = layerMap.find("Collision");
    if (it != layerMap.end())
        m_collision = std::move(it->second);

    return !m_visuals.empty() && !m_collision.empty();
}

bool Map::load(const std::string& tmxPath) {
    return parseTMX(tmxPath);
}

const Map::Tileset* Map::findTileset(int gid) const {
    for (const auto& ts : m_tilesets)
        if (gid >= ts.firstgid) return &ts;
    return nullptr;
}

void Map::drawGrid(sf::RenderTarget& target, const Grid& grid) {
    static const uint32_t FLAG_H = 0x80000000u;
    static const uint32_t FLAG_V = 0x40000000u;
    static const uint32_t FLAG_D = 0x20000000u;

    sf::Sprite sprite;
    for (int y = 0; y < (int)grid.size(); ++y) {
        for (int x = 0; x < (int)grid[y].size(); ++x) {
            uint32_t raw = grid[y][x];
            if (raw == 0) continue;

            bool flipH = (raw & FLAG_H) != 0;
            bool flipV = (raw & FLAG_V) != 0;
            bool flipD = (raw & FLAG_D) != 0;
            int gid = (int)(raw & 0x1FFFFFFFu);

            const Tileset* ts = findTileset(gid);
            if (!ts || ts->cols == 0) continue;
            int local = gid - ts->firstgid;

            sprite.setTexture(ts->texture);
            sprite.setTextureRect({
                (local % ts->cols) * TILE_SIZE,
                (local / ts->cols) * TILE_SIZE,
                TILE_SIZE, TILE_SIZE
            });

            float rotation = 0.f;
            sf::Vector2f scale(1.f, 1.f);
            if (flipD) {
                if (flipH && flipV) { rotation =  90.f; scale.y = -1.f; }
                else if (flipH)     { rotation =  90.f; }
                else if (flipV)     { rotation = -90.f; }
                else                { rotation = -90.f; scale.x = -1.f; }
            } else {
                if (flipH) scale.x = -1.f;
                if (flipV) scale.y = -1.f;
            }

            sprite.setOrigin(TILE_SIZE / 2.f, TILE_SIZE / 2.f);
            sprite.setRotation(rotation);
            sprite.setScale(scale);
            sprite.setPosition(x * TILE_SIZE + TILE_SIZE / 2.f,
                               y * TILE_SIZE + TILE_SIZE / 2.f);
            target.draw(sprite);
        }
    }
}

void Map::draw(sf::RenderTarget& target) {
    for (const auto& layer : m_visuals)
        drawGrid(target, layer);
}

bool Map::isSolid(int tileX, int tileY) const {
    if (tileY < 0 || tileY >= (int)m_collision.size()) return true;
    if (tileX < 0 || tileX >= (int)m_collision[tileY].size()) return true;
    return (m_collision[tileY][tileX] & 0x1FFFFFFFu) > 0;
}

bool Map::isCollidingWith(const sf::FloatRect& bounds) const {
    int left   = (int)(bounds.left)                     / TILE_SIZE;
    int right  = (int)(bounds.left + bounds.width  - 1) / TILE_SIZE;
    int top    = (int)(bounds.top)                      / TILE_SIZE;
    int bottom = (int)(bounds.top  + bounds.height - 1) / TILE_SIZE;
    for (int y = top; y <= bottom; ++y)
        for (int x = left; x <= right; ++x)
            if (isSolid(x, y)) return true;
    return false;
}
