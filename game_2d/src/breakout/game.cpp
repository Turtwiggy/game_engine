
//header
#include "breakout/game.hpp"

//C++ lib headers
#include <iostream>
#include <fstream>
#include <sstream>

//your lib headers
#include "engine/resources/resource_manager.hpp"

namespace game2d
{


GameObject::GameObject(Texture2D* tex)
    : texture(tex)
{

}

Ball::Ball(Texture2D* tex)
    : game_object( tex )
{
    printf("creating ball!");
}

void reset_ball( Ball& ball )
{

}

void move_ball( Ball& ball, float delta_time_s, int window_width )
{
    if( ! ball.stuck )
    {
        Transform& transform = ball.game_object.transform;
        transform.position += ball.game_object.velocity * delta_time_s;

        //if ball goes off left of screen...
        if (transform.position.x <= 0.0f)
        {
            ball.game_object.velocity.x = glm::abs(ball.game_object.velocity.x);
            transform.position.x = 0.0f;
        }
        //if ball goes off right of screen...
        else if (transform.position.x + transform.scale.x >= window_width)
        {
            ball.game_object.velocity.x = -glm::abs(ball.game_object.velocity.x);
            transform.position.x = window_width - transform.scale.x;
        }
        // if ball goes off top of screen...
        if (transform.position.y <= 0.0f)
        {
            ball.game_object.velocity.y = glm::abs(ball.game_object.velocity.y);
            transform.position.y = 0.0f;
        }
    }
}

// ---- level load functions

void load_level_from_file( std::vector<std::vector<int>>& layout, const std::string &path )
{
    std::ifstream fstream(path);

    if (fstream)
    {
        int tile_code;
        std::string line;
        while (std::getline(fstream, line))
        {
            //std::cout << line << "\n";

            std::istringstream sstream(line);
            std::vector<int> row;

            while (sstream >> tile_code)
            {
                //std::cout << tile_code << "\n";

                row.push_back(tile_code);
            }

            if(row.size() > 0)
                layout.push_back(row);
        }
    }
    else
    {
        std::cout << "error reading game level file! \n";
    }
};

void init_level( GameLevel& level, const std::vector<std::vector<int>>& layout, int level_width, int level_height )
{
    level.bricks.clear();

    int height = static_cast<int>(layout.size());
    int width = static_cast<int>(layout[0].size());
    //printf("level, h: %i w: %i \n", height, width);

    float unit_width = level_width / static_cast<float>(width);
    float unit_height = level_height / height; 

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int tile_data = layout[y][x];

            glm::vec2 pos(unit_width * x, unit_height * y);
            glm::vec2 size(unit_width, unit_height);
            //printf("tile_data: %i pos x: %f y: %f size x: %f y: %f \n", tile_data, pos.x, pos.y, size.x, size.y);
            
            if( tile_data == 1 )
            {
                GameObject go { fightingengine::ResourceManager::get_texture( "block_solid" ) };
                
                //transform
                go.transform.position = pos;
                go.transform.scale = size;
                go.transform.colour = {0.8f, 0.8f, 0.7f};
                go.transform.angle = 0.0f;
                //state
                go.is_solid = true;

                level.bricks.push_back(go);
            } 
            else
            {
                GameObject go { fightingengine::ResourceManager::get_texture( "block" ) };

                glm::vec3 color = glm::vec3(1.0f); // original: white
                if (tile_data == 2)
                    color = glm::vec3(0.2f, 0.6f, 1.0f); //blueish
                else if (tile_data == 3)
                    color = glm::vec3(0.0f, 0.7f, 0.0f); //greenish
                else if (tile_data == 4)
                    color = glm::vec3(0.8f, 0.8f, 0.4f); 
                else if (tile_data == 5)
                    color = glm::vec3(1.0f, 0.5f, 0.0f);

                //transform
                go.transform.position = pos;
                go.transform.scale = size;
                go.transform.colour = color;
                go.transform.angle = 0.0f;
                //state
                go.is_solid = false;

                level.bricks.push_back(go);
            }
        }
    }
}

// ---- simple aabb collisions

bool has_collided( GameObject& one, GameObject& two )
{
    Transform& t1 = one.transform;
    Transform& t2 = two.transform;

    // collision x-axis?
    bool collided_x = 
          t1.position.x + t1.scale.x >= t2.position.x &&
          t2.position.x + t2.scale.x >= t1.position.x;

    // collision y-axis?
    bool collided_y = 
        t1.position.y + t1.scale.y >= t2.position.y &&
        t2.position.y + t2.scale.y >= t1.position.y;

    // collision only if on both axes
    return collided_x && collided_y;
}

CollisionDirection get_collision_direction( glm::vec2 target )
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),	// up
        glm::vec2(1.0f, 0.0f),	// right
        glm::vec2(0.0f, -1.0f),	// down
        glm::vec2(-1.0f, 0.0f)	// left
    };
    float max = 0.0f;
    int best_match = -1;
    for ( int i = 0; i < 4; i++ )
    {
        float dot_product = glm::dot( glm::normalize( target ), compass[i] );
        if ( dot_product > max )
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (CollisionDirection)best_match;
}

CollisionInfo has_collided( Ball& ball, GameObject& other )
{
    glm::vec2 center( ball.game_object.transform.position + ball.radius );

    glm::vec2 aabb_half_extents(
            other.transform.scale.x / 2.0f, 
            other.transform.scale.y / 2.0f );

    glm::vec2 aabb_center(
        other.transform.position.x + aabb_half_extents.x,
        other.transform.position.y + aabb_half_extents.y );

    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp( difference, -aabb_half_extents, aabb_half_extents );
    glm::vec2 closest = aabb_center + clamped;

    difference = closest - center;

    if ( glm::length( difference ) <= ball.radius ) 
        return std::make_tuple( true, get_collision_direction( difference ), difference );
    else
        return std::make_tuple( false, CollisionDirection::COLLISION_UP, glm::vec2{ 0.0f, 0.0f } );
}

void do_collisions_bricks( GameLevel& objects, Ball& ball )
{
    for ( auto& box : objects.bricks )
    {
        if ( ! box.destroyed )
        {
            CollisionInfo collision = has_collided( ball, box );

            if ( !std::get<0>(collision) ) // no collision
                continue;
            
            if( ! box.is_solid )
                box.destroyed = true;

            // collision resolution
            CollisionDirection dir = std::get<1>(collision);
            glm::vec2 difference = std::get<2>(collision);

            //horizontal collision
            if ( dir == CollisionDirection::COLLISION_LEFT || dir == CollisionDirection::COLLISION_RIGHT )
            {
                ball.game_object.velocity.x = -ball.game_object.velocity.x;

                //relocate ball to collision point
                float depth = ball.radius - glm::abs( difference.x );
                if( dir == CollisionDirection::COLLISION_LEFT )
                    ball.game_object.transform.position.x += depth; //move ball right
                else
                    ball.game_object.transform.position.x = depth;  //move ball left
            }
            //vertical collision
            else
            {
                ball.game_object.velocity.y = -ball.game_object.velocity.y;

                float depth = ball.radius - glm::abs( difference.y );
                if( dir == CollisionDirection::COLLISION_UP)
                    ball.game_object.transform.position.y -= depth; //move ball up
                else
                    ball.game_object.transform.position.y += depth; //move ball down
            }
        }
    }
}

void do_collisions_player( GameObject& player, Ball& ball )
{
    CollisionInfo info = has_collided(ball, player);

    if( ! ball.stuck && std::get<0>(info) )
    {
       // printf("ball collided with paddle!");
        
        float center = player.transform.position.x + player.transform.scale.x / 2.0f;
        float distance = ball.game_object.transform.position.x + ball.radius - center;
        float percentage = distance / player.transform.scale.x / 2.0f;
        //printf("percentage: %f", percentage);

        float strength = 2.0f;
        glm::vec2 old_velocity = ball.game_object.velocity;
        float initial_ball_velocity = 100.0f;
        ball.game_object.velocity.x = initial_ball_velocity * percentage * strength;
        ball.game_object.velocity.y = -glm::abs(ball.game_object.velocity.y);
        ball.game_object.velocity = glm::normalize( ball.game_object.velocity ) * glm::length(old_velocity);
    }
}

// ---- breakout game functions

void update_user_input( Application& app, float delta_time_s, GameObject& player, Ball& ball, float screen_width )
{
    if( app.get_input().get_key_held( SDL_SCANCODE_A ) )
    {
        float velocity_x =  player.velocity.x * delta_time_s;
        if ( player.transform.position.x >= 0.0f )
        {
            player.transform.position.x -= velocity_x;

            if( ball.stuck )
                ball.game_object.transform.position.x -= velocity_x;
        }
    }
    if( app.get_input().get_key_held( SDL_SCANCODE_D ) )
    {
        float velocity_x =  player.velocity.x * delta_time_s;
        if (player.transform.position.x <= screen_width - player.transform.scale.x )
        {
            player.transform.position.x += velocity_x;

            if( ball.stuck )
                ball.game_object.transform.position.x += velocity_x;
        }
    }
    if( app.get_input().get_key_down( SDL_KeyCode::SDLK_SPACE ) )
    {
        printf("ball unstuck \n");
        ball.stuck = false;
    }
}

void update_game_state()
{

}

} //namespace game2d
