#define PIXELDRAW_IMPLEMENTATION
#include "PixelDraw.hh"

#include <vector>
#include <cmath>
#include <list>

#define PI 3.14159f

class Raycaster : public mrt::PixelDraw {
private:
    std::string data_path;

    mrt::vec2f player;
    float player_angle = 0.0f;
    float fov = PI / 2.5;
    float depth = 30.0;

    float sample_x = 0.0f;

    // Map
    int map_width = 24;
    int map_height = 24;

    std::vector<int> map {
        8,8,8,8,8,8,8,8,8,8,8,4,4,6,4,4,6,4,6,4,4,4,6,4,
        8,0,0,0,0,0,0,0,0,0,8,4,0,0,0,0,0,0,0,0,0,0,0,4,
        8,0,3,3,0,0,0,0,0,8,8,4,0,0,0,0,0,0,0,0,0,0,0,6,
        8,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,
        8,0,3,3,0,0,0,0,0,8,8,4,0,0,0,0,0,0,0,0,0,0,0,4,
        8,0,0,0,0,0,0,0,0,0,8,4,0,0,0,0,0,6,6,6,0,6,4,6,
        8,8,8,8,0,8,8,8,8,8,8,4,4,4,4,4,4,6,0,0,0,0,0,6,
        7,7,7,7,0,7,7,7,7,0,8,0,8,0,8,0,8,4,0,4,0,6,0,6,
        7,7,0,0,0,0,0,0,7,8,0,8,0,8,0,8,8,6,0,0,0,0,0,6,
        7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,6,0,0,0,0,0,4,
        7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,6,0,6,0,6,0,6,
        7,7,0,0,0,0,0,0,7,8,0,8,0,8,0,8,8,6,4,6,0,6,6,6,
        7,7,7,7,0,7,7,7,7,8,8,4,0,6,8,4,8,3,3,3,0,3,3,3,
        2,2,2,2,0,2,2,2,2,4,6,4,0,0,6,0,6,3,0,0,0,0,0,3,
        2,2,0,0,0,0,0,2,2,4,0,0,0,0,0,0,4,3,0,0,0,0,0,3,
        2,0,0,0,0,0,0,0,2,4,0,0,0,0,0,0,4,3,0,0,0,0,0,3,
        1,0,0,0,0,0,0,0,1,4,4,4,4,4,6,0,6,3,3,0,0,0,3,3,
        2,0,0,0,0,0,0,0,2,2,2,1,2,2,2,6,6,0,0,5,0,5,0,5,
        2,2,0,0,0,0,0,2,2,2,0,0,0,2,2,0,5,0,5,0,0,0,5,5,
        2,0,0,0,0,0,0,0,2,0,0,0,0,0,2,5,0,5,0,5,0,5,0,5,
        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,
        2,0,0,0,0,0,0,0,2,0,0,0,0,0,2,5,0,5,0,5,0,5,0,5,
        2,2,0,0,0,0,0,2,2,2,0,0,0,2,2,0,5,0,5,0,0,0,5,5,
        2,2,2,2,1,2,2,2,2,2,2,1,2,2,2,5,5,5,5,5,5,5,5,5

    };

    // Parameters
    float step = 0.01f;
    int texture_column_width = 1;
    int floor_scale = 2;

    float rotation_speed = 3.0f;
    float movement_speed = 4.0f;

    // GameObject
    struct GameObject {
        mrt::vec2f pos;         // Position
        mrt::vec2f v;           // Velocity
        bool remove;
        mrt::Texture *texture;
    };

    SDL_Rect texture_source, texture_dest;
    float *depth_buffer = nullptr;

    // Resources
    SDL_Texture* buffer = nullptr;
    std::vector<mrt::Texture> textures;
    std::list<std::pair<int, GameObject>> objects;

private:
    inline int get_map_tile(int x, int y) {
        return map[y * map_width + x];
    }

    inline int get_map_tile(const mrt::vec2i& v) {
        return map[v.y * map_width + v.x];
    }

public:
    Raycaster(const std::string& data_path) : PixelDraw("Raycaster", 640, 480), player(8.0, 8.0), data_path(data_path) {
        depth_buffer = new float[get_width()];

        if (this->data_path[data_path.size()-1] != '/') {
            this->data_path += '/';
        }

        // set_fps_cap(60);
        buffer = SDL_CreateTexture(get_renderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, get_width(), get_height());
    }

    ~Raycaster() {
        INFO("Unloading raycaster resources.");
        delete [] depth_buffer;
        SDL_DestroyTexture(buffer);
    }

    void on_load() override {
        textures.push_back(create_texture(data_path + "res/logo.png"));
        textures.push_back(create_texture(data_path + "res/wolf3d/WALL91.bmp"));
        textures.push_back(create_texture(data_path + "res/wolf3d/WALL0.bmp"));
        textures.push_back(create_texture(data_path + "res/wolf3d/WALL4.bmp"));
        textures.push_back(create_texture(data_path + "res/wolf3d/WALL10.bmp"));
        textures.push_back(create_texture(data_path + "res/wolf3d/WALL22.bmp"));
        textures.push_back(create_texture(data_path + "res/wolf3d/WALL20.bmp"));
        textures.push_back(create_texture(data_path + "res/wolf3d/WALL18.bmp"));
        textures.push_back(create_texture(data_path + "res/wolf3d/WALL44.bmp"));
        textures.push_back(create_texture(data_path + "res/sprites/barrel.png"));
        textures.push_back(create_texture(data_path + "res/sprites/pillar.png"));
        textures.push_back(create_texture(data_path + "res/fireball.png"));

        objects = {
            {0, {{20.5f, 2.5f}, {0.0f, 0.0f}, false, &textures[9]}},
            {0, {{5.5f,  2.5f}, {0.0f, 0.0f}, false, &textures[9]}},
            {0, {{4.5f, 20.5f}, {0.0f, 0.0f}, false, &textures[10]}},
            {0, {{11.5f,20.5f}, {0.0f, 0.0f}, false, &textures[10]}},
        };
    }

    void on_frame_update(float frame_time) override {
        int screen_width = get_width();
        int screen_height = get_height();

        // Movement
        if (get_key_state(SDL_SCANCODE_LEFT).held) {
            player_angle -= rotation_speed * frame_time;
        }

        if (get_key_state(SDL_SCANCODE_RIGHT).held) {
            player_angle += rotation_speed * frame_time;
        }

        if (get_key_state(SDL_SCANCODE_W).held) {
            player.x += sinf(player_angle) * movement_speed * frame_time;
            player.y += cosf(player_angle) * movement_speed * frame_time;

            if (get_map_tile((int)player.x, (int)player.y) != 0) {
                player.x -= sinf(player_angle) * movement_speed * frame_time;
                player.y -= cosf(player_angle) * movement_speed * frame_time;
            }
        }

        if (get_key_state(SDL_SCANCODE_S).held) {
            player.x -= sinf(player_angle) * movement_speed * frame_time;
            player.y -= cosf(player_angle) * movement_speed * frame_time;

            if (get_map_tile((int)player.x, (int)player.y) != 0) {
                player.x += sinf(player_angle) * movement_speed * frame_time;
                player.y += cosf(player_angle) * movement_speed * frame_time;
            }
        }

        if (get_key_state(SDL_SCANCODE_A).held) {
            player.x -= cosf(player_angle) * movement_speed * frame_time;
            player.y += sinf(player_angle) * movement_speed * frame_time;

            if (get_map_tile((int)player.x, (int)player.y) != 0) {
                player.x += cosf(player_angle) * movement_speed * frame_time;
                player.y -= sinf(player_angle) * movement_speed * frame_time;
            }
        }

        if (get_key_state(SDL_SCANCODE_D).held) {
            player.x += cosf(player_angle) * movement_speed * frame_time;
            player.y -= sinf(player_angle) * movement_speed * frame_time;

            if (get_map_tile((int)player.x, (int)player.y) != 0) {
                player.x -= cosf(player_angle) * movement_speed * frame_time;
                player.y += sinf(player_angle) * movement_speed * frame_time;
            }
        }

        if (get_key_state(SDL_SCANCODE_SPACE).pressed) {
            GameObject o;
            o.pos = player;
            o.v = {sinf(player_angle) * 5, cosf(player_angle) * 5};
            o.remove = false;
            o.texture = &textures[11];
            objects.push_back({0, o});
        }

        // Set buffer as a rendering target
        SDL_SetRenderTarget(get_renderer(), buffer);
        SDL_RenderClear(get_renderer());

        // Solid floor rendering 
        texture_dest.x = 0;
        texture_dest.y = screen_height/2;
        texture_dest.w = screen_width;
        texture_dest.h = screen_height/2;

        SDL_SetRenderDrawColor(get_renderer(), 128, 128, 128, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(get_renderer(), &texture_dest);

        /*
        mrt::vec2f ray0(
            sinf(player_angle-fov/2.0f),
            cosf(player_angle-fov/2.0f)
        );
        mrt::vec2f ray1(
            sinf(player_angle+fov/2.0f),
            cosf(player_angle+fov/2.0f)
        );

        SDL_Texture* floor_texture = textures[1].get_sdl_texture();

        int floor_texture_width = textures[1].get_width();
        int floor_texture_height = textures[1].get_height();

        texture_source.w = floor_scale;
        texture_source.h = floor_scale;
        texture_dest.w = floor_scale;
        texture_dest.h = floor_scale;

        // Floor Casting
        for (int y = screen_height/2; y < screen_height; y+=floor_scale) {
            int y_pos = y - screen_height/2;
            float z = screen_height/2.0f;

            float row_distance = z / y_pos;

            mrt::vec2f floor_step(
                floor_scale * row_distance * (ray1.x - ray0.x) / screen_width,
                floor_scale * row_distance * (ray1.y - ray0.y) / screen_width
            );

            mrt::vec2f floor(
                player.x + row_distance * ray0.x,
                player.y + row_distance * ray0.y
            );

            texture_dest.y = y;

            for (texture_dest.x = 0; texture_dest.x < screen_width; texture_dest.x+=floor_scale) {
                mrt::vec2i texture_coords(
                    (int)(floor_texture_width * (floor.x - (int)floor.x)) & (floor_texture_width-1),
                    (int)(floor_texture_height * (floor.y - (int)floor.y)) & (floor_texture_height-1)
                );

                floor.x += floor_step.x;
                floor.y += floor_step.y;

                texture_source.x = texture_coords.x;
                texture_source.y = texture_coords.y;

                SDL_RenderCopy(get_renderer(), floor_texture, &texture_source, &texture_dest);
            }
        }*/

        // Wall Rendering
        for (int x = 0; x < screen_width; x+=texture_column_width) {
            float ray_angle = (player_angle - fov/2.0f) + ((float)x / (float)screen_width) * fov;
            float distance_to_wall = 0;

            bool hit_wall = false;

            mrt::vec2f eye(
                sinf(ray_angle),
                cosf(ray_angle)
            );

            mrt::vec2i test(0, 0);
            int side = 0;

            while (!hit_wall && distance_to_wall < depth) {
                distance_to_wall += step;

                test.x = player.x + eye.x * distance_to_wall;
                test.y = player.y + eye.y * distance_to_wall;

                if (test.x < 0 || test.x > map_width || test.y < 0 || test.y > map_height) {
                    hit_wall = true;
                    distance_to_wall = depth;
                } else {
                    if (get_map_tile(test) != 0) {
                        hit_wall = true;

                        mrt::vec2f block_mid(
                            test.x + 0.5f,
                            test.y + 0.5f
                        );
                        mrt::vec2f test_point(
                            player.x + eye.x * distance_to_wall,
                            player.y + eye.y * distance_to_wall
                        );

                        float test_angle = atan2f((test_point.y - block_mid.y), (test_point.x - block_mid.x));

                        if (test_angle >= -PI * 0.25f && test_angle < PI * 0.25f) {
                            sample_x = test_point.y - test.y;
                            side = 1;
                        }
                        if (test_angle >= PI * 0.25f && test_angle < PI * 0.75f) {
                            sample_x = test_point.x - test.x;
                            side = 0;
                        }
                        if (test_angle < -PI * 0.25f && test_angle >= -PI * 0.75f) {
                            sample_x = test_point.x - test.x;
                            side = 0;
                        }
                        if (test_angle >= PI * 0.75f || test_angle < -PI * 0.75f) {
                            sample_x = test_point.y - test.y;
                            side = 1;
                        }

                        distance_to_wall = distance_to_wall * cosf(ray_angle-player_angle);
                    }
                }
            }

            // int ceiling = (float)(screen_height / 2.0f) - screen_height / ((float)distance_to_wall);
            // int floor = screen_height - ceiling;
            // unsigned char shade = 255 * (1 - distance_to_wall/depth);
            // SDL_SetRenderDrawColor(get_renderer(), shade, shade, shade, SDL_ALPHA_OPAQUE);
            // SDL_RenderDrawLine(get_renderer(), x, ceiling, x+1, floor);

            int y_start = (float)(screen_height / 2.0f) - screen_height / ((float)distance_to_wall) / 2.0;
            int y_end = y_start + screen_height/distance_to_wall;

            depth_buffer[x] = distance_to_wall;

            mrt::Texture& texture = textures.at(get_map_tile(test));

            float whole;

            texture_source.x = (std::modf(sample_x, &whole) * texture.get_width());
            texture_source.y = 0;
            texture_source.w = texture_column_width;
            texture_source.h = texture.get_height();

            texture_dest.x = x;
            texture_dest.y = y_start;
            texture_dest.w = texture_column_width;
            texture_dest.h = (float)screen_height/distance_to_wall;

            SDL_RenderCopy(get_renderer(), texture.get_sdl_texture(), &texture_source, &texture_dest);
        }

        // Sprites rendering
        for (auto &object : objects) {
            object.second.pos.x += object.second.v.x * frame_time;
            object.second.pos.y += object.second.v.y * frame_time;

            if (get_map_tile(object.second.pos.x, object.second.pos.y) != 0) {
                object.second.remove = true;
            }

            mrt::vec2f vec(
                object.second.pos.x - player.x,
                object.second.pos.y - player.y
            );

            float distance_from_player = sqrtf(vec.x*vec.x + vec.y*vec.y);

            object.first = distance_from_player;

            mrt::vec2f eye(
                sinf(player_angle),
                cosf(player_angle)
            );

            float object_angle = atan2f(eye.y, eye.x) - atan2f(vec.y, vec.x);
            if (object_angle < -PI)
                object_angle += 2.0f * PI;
            if (object_angle > PI)
                object_angle -= 2.0f * PI;

            bool is_in_fov = fabs(object_angle) < fov / 2.0f;

            if (is_in_fov && distance_from_player >= 0.5f && distance_from_player < depth) {
                float object_ceiling = (float)(screen_height / 2.0f) - screen_height/distance_from_player/1.5;
                float object_floor = screen_height - object_ceiling;
                float object_height = object_floor - object_ceiling;
                float object_aspect_ratio = (float)object.second.texture->get_height() / (float)object.second.texture->get_width();
                float object_width = object_height/object_aspect_ratio;
                float object_middle = (0.5f * (object_angle / (fov / 2.0f)) + 0.5f) * (float)screen_width;

                SDL_Rect texture_source, texture_dest;

                float whole;

                for (int sx = 0; sx < object_width; sx++) {
                    int object_column = object_middle + sx - (object_width/2.0f);

                    texture_source.x = modf(sx / object_width, &whole) * object.second.texture->get_width();
                    texture_source.y = 0;
                    texture_source.w = 1;
                    texture_source.h = object.second.texture->get_height();

                    texture_dest.x = object_column;
                    texture_dest.y = object_ceiling;
                    texture_dest.w = 1;
                    texture_dest.h = object_height;

                    if (depth_buffer[object_column] >= distance_from_player) {
                        SDL_RenderCopy(get_renderer(), object.second.texture->get_sdl_texture(), &texture_source, &texture_dest);
                        // depth_buffer[object_column] = distance_from_player;
                    }
                }
            }
        }

        SDL_SetRenderTarget(get_renderer(), NULL);
        SDL_RenderCopy(get_renderer(), buffer, NULL, NULL);

        // Remove objects that shuold be removed
        objects.remove_if([](std::pair<int, GameObject>& p) { return p.second.remove; });

        // Sort object by distance from player
        objects.sort([](const std::pair<int, GameObject> a, const std::pair<int, GameObject> b) { return a.first >  b.first; });
    }
};

int main(int argc, char ** argv) {
    const char* datapath = nullptr;
    if (argc != 2) {
        ERROR("Please provide data folder path.");
        return 1;
    }
    datapath = argv[1];
    Raycaster raycaster(datapath);
    raycaster.run();
    return 0;
}