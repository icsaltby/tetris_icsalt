#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <cstdint>
#include <array>

// 添加游戏状态枚举
enum class GameState {
    MAIN_MENU,
    INSTRUCTIONS,
    DIFFICULTY_SELECT,
    PLAYING,
    GAME_OVER
};

// 添加难度级别枚举
enum class Difficulty {
    EASY,
    MEDIUM,
    HARD
};

class Tetris {

  static const std::uint32_t lines   {20};
  static const std::uint32_t cols    {10};
  static const std::uint32_t squares {4};
  static const std::uint32_t shapes  {7};

  // 新增炸弹相关成员
  int bombs;                      // 当前拥有的炸弹数量
  sf::Texture bombTexture;        // 炸弹图标纹理
  sf::Sprite bombSprite;          // 炸弹图标精灵
  sf::Text bombsText;             // 炸弹数量显示

  // 游戏状态相关
  GameState currentState;
  Difficulty currentDifficulty;
    
  // 新增菜单相关资源
  sf::Texture menuBgTexture;
  sf::Sprite menuBgSprite;
  sf::Texture instructionsTexture;
  sf::Sprite instructionsSprite;
  sf::Texture difficultyTexture;
  sf::Sprite difficultySprite;
    
  // 菜单按钮区域定义
  struct MenuButton {
      sf::IntRect rect;
      GameState targetState;
  };
    
  std::array<MenuButton, 3> mainMenuButtons;
  std::array<MenuButton, 3> difficultyButtons;
    
  // 难度设置
  std::array<float, 3> difficultySpeeds {0.5f, 0.3f, 0.1f}; // 简单、中等、困难的下落速度

  std::vector<std::vector<std::uint32_t>> area;
  std::vector<std::vector<std::uint32_t>> forms;

  struct Coords {
    std::uint32_t x, y;
  } z[squares], k[squares];

  std::shared_ptr<sf::RenderWindow> window;
  sf::Texture tiles, bg;
  std::shared_ptr<sf::Sprite> sprite, background;
  sf::Clock clock;
  sf::Font  font;
  sf::Text txtScore, txtGameOver;

  bool aiEnabled;
  void simpleAI();  // AI逻辑
  int evaluateFlatness() const; // 评估平整度

  int dirx, color, score;
  bool rotate, gameover;
  float timercount, delay;

  protected:
    void events();
    void draw();
    void moveToDown();
    void setRotate();
    void resetValues();
    void changePosition();
    bool maxLimit();
    void setScore();
    void loadMenuResources();
    void drawMainMenu();
    void drawInstructions();
    void drawDifficultySelect();
    void setupMenuButtons();
    void useBomb();                 // 使用炸弹
    void updateBombsDisplay();      // 更新炸弹显示
    void simulateHardDrop();

  public:
    Tetris();
    ~Tetris();
    void run();
};
