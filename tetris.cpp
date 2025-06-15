#include "tetris.hpp"
#include <cstdlib>
#include <string> 

Tetris::Tetris(){

  area.resize(lines);
  for (std::size_t i {}; i < area.size(); ++i) {
    area[i].resize(cols); 
  }

  forms = {
    {1,3,5,7}, // I
    {2,4,5,7}, // Z
    {3,5,4,6}, // S
    {3,5,4,7}, // T
    {2,3,5,7}, // L
    {3,5,7,6}, // J
    {2,3,4,5}, // O
  };

  window = std::make_shared<sf::RenderWindow>(
      sf::VideoMode(360, 720),
      "Tetris (remix)",
      sf::Style::Titlebar | sf::Style::Close
      );
  window->setPosition(sf::Vector2i(100, 100));

  tiles.loadFromFile("../../src/resources/img/squares.png");
  sprite = std::make_shared<sf::Sprite>();
  sprite->setTexture( tiles );
  sprite->setTextureRect(sf::IntRect(0, 0, 36, 36));

  bg.loadFromFile("../../src/resources/img/background.png");
  background = std::make_shared<sf::Sprite>();
  background->setTexture(bg);

  // 初始化游戏状态
  currentState = GameState::MAIN_MENU;
  currentDifficulty = Difficulty::MEDIUM; // 默认中等难度
  resetValues(); // 这会根据难度设置初始delay
  // 加载菜单资源
  loadMenuResources();
  setupMenuButtons();

  if (!bombTexture.loadFromFile("../../src/resources/img/bomb.png")) {
      // 创建简单的炸弹图标作为后备
      bombTexture.create(36, 36);
      // 可以添加简单的绘制代码创建默认炸弹图标
  }
  bombSprite.setTexture(bombTexture);
  bombSprite.setPosition(280.f, 10.f); // 右上角显示
    
  bombsText.setFont(font);
  bombsText.setPosition(320.f, 10.f);
  bombsText.setCharacterSize(30);
  bombsText.setOutlineThickness(3);
  updateBombsDisplay();

  dirx = score = {0};
  rotate = gameover = {false};
  timercount = {0.f};
  delay = {0.3f};
  color = {1};

  std::uint32_t number = std::rand() % shapes;
  for (std::size_t i {}; i < squares; ++i) {
    z[i].x = forms[number][i] % 2; 
    z[i].y = forms[number][i] / 2; 
  }

  font.loadFromFile("../../src/resources/font/font.ttf");
  txtScore.setFont(font);
  txtScore.setPosition(100.f,10.f);
  txtScore.setString("SCORE: " + std::to_string(score));
  txtScore.setCharacterSize(30);
  txtScore.setOutlineThickness(3);

  txtGameOver.setFont(font);
  txtGameOver.setPosition(30.f,300.f);
  txtGameOver.setString("GAME OVER");
  txtGameOver.setCharacterSize(50);
  txtGameOver.setOutlineThickness(3);
}

Tetris::~Tetris() {
    if(window && window->isOpen()) window->close(); 
    // 无需手动释放shared_ptr，但确保窗口关闭
}

void Tetris::loadMenuResources() {
    // 加载菜单背景
    if (!menuBgTexture.loadFromFile("../../src/resources/img/menu_bg.png")) {
        // 处理加载失败
    }
    menuBgSprite.setTexture(menuBgTexture);
    
    // 加载说明界面
    if (!instructionsTexture.loadFromFile("../../src/resources/img/instructions.png")) {
        // 处理加载失败
    }
    instructionsSprite.setTexture(instructionsTexture);
    
    // 加载难度选择界面
    if (!difficultyTexture.loadFromFile("../../src/resources/img/difficulty_select.png")) {
        // 处理加载失败
    }
    difficultySprite.setTexture(difficultyTexture);
}

void Tetris::setupMenuButtons() {
    // 主菜单按钮区域 (根据你的图片调整这些坐标)
    mainMenuButtons[0] = {{135, 324, 90, 36}, GameState::INSTRUCTIONS};    // 说明
    mainMenuButtons[1] = {{135, 252, 90, 36}, GameState::DIFFICULTY_SELECT}; // 难度选择
    mainMenuButtons[2] = {{135, 180, 90, 36}, GameState::PLAYING};         // 开始游戏
    
    // 难度选择按钮区域
    difficultyButtons[0] = {{135, 144, 90, 36}, GameState::PLAYING}; // 简单
    difficultyButtons[1] = {{135, 252, 90, 36}, GameState::PLAYING}; // 中等
    difficultyButtons[2] = {{135, 360, 90, 36}, GameState::PLAYING}; // 困难
}

void Tetris::drawMainMenu() {
    window->clear();
    window->draw(menuBgSprite);
    window->display();
}

void Tetris::drawInstructions() {
    window->clear();
    window->draw(instructionsSprite);
    window->display();
}

void Tetris::drawDifficultySelect() {
    window->clear();
    window->draw(difficultySprite);
    window->display();
}

void Tetris::events() {
    sf::Event e;
    while (window->pollEvent(e)) {
        // 通用事件 - 所有状态都响应
        if (e.type == sf::Event::Closed) {
            window->close();
            return;
        }

        // 游戏进行时的键盘控制 (PLAYING状态且游戏未结束)
        if (currentState == GameState::PLAYING && !gameover) {
            if (e.type == sf::Event::KeyPressed) {
                switch (e.key.code) {
                    case sf::Keyboard::Up:    rotate = true;   break;
                    case sf::Keyboard::Right: ++dirx;         break;
                    case sf::Keyboard::Left:  --dirx;         break;
                    case sf::Keyboard::Space: useBomb();      break;// 空格键使用炸弹
                    case sf::Keyboard::A: aiEnabled = !aiEnabled;     break;
                    case sf::Keyboard::Escape: 
                        currentState = GameState::MAIN_MENU;  // 游戏内按ESC返回菜单
                        break;
                    default: break;
                }
            }
        }

        // 菜单系统的鼠标控制 (非PLAYING状态)
        if (currentState != GameState::PLAYING) {
            // 鼠标点击处理
            if (e.type == sf::Event::MouseButtonPressed && 
                e.mouseButton.button == sf::Mouse::Left) {
                
                sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
                
                // 主菜单按钮
                if (currentState == GameState::MAIN_MENU) {
                    for (const auto& button : mainMenuButtons) {
                        if (button.rect.contains(mousePos)) {
                            currentState = button.targetState;
                            if (currentState == GameState::PLAYING) {
                                resetValues(); // 开始游戏时重置状态
                            }
                            break;
                        }
                    }
                }
                // 难度选择按钮
                else if (currentState == GameState::DIFFICULTY_SELECT) {
                    for (size_t i = 0; i < difficultyButtons.size(); ++i) {
                        if (difficultyButtons[i].rect.contains(mousePos)) {
                            currentDifficulty = static_cast<Difficulty>(i);
                            // 这里调用resetValues()来设置正确的delay
                            resetValues(); 
                            currentState = GameState::PLAYING;
                            break;
                        }
                    }
                }
                // 说明界面 - 点击任意位置返回
                else if (currentState == GameState::INSTRUCTIONS) {
                    currentState = GameState::MAIN_MENU;
                }
            }
            
            // 菜单状态下ESC键返回主菜单
            if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) {
                currentState = GameState::MAIN_MENU;
            }
        }
    }

    // 持续按键检测 - 仅游戏进行时有效
    if (currentState == GameState::PLAYING && !gameover) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            delay = 0.05f; // 加速下落
        }
    }
}

void Tetris::draw(){
  window->clear(sf::Color::Black);
  window->draw(*background);

  for (std::size_t i {}; i < lines; ++i) {
   for (std::size_t j {}; j < cols; ++j) {
     if( area[i][j] != 0 ){
       sprite->setTextureRect(sf::IntRect( area[i][j] * 36, 0, 36, 36 ));
       sprite->setPosition(j * 36, i * 36);
       window->draw(*sprite);

     }
   } 
  }

  for (std::size_t i {}; i < squares; ++i) {
    sprite->setTextureRect(sf::IntRect( color * 36, 0, 36, 36 ));
    sprite->setPosition( z[i].x * 36, z[i].y * 36);
    window->draw(*sprite);
  }

  // 绘制炸弹图标和数量
  if (currentState == GameState::PLAYING) {
      window->draw(bombSprite);
      window->draw(bombsText);
  }

  window->draw(txtScore);
  if( gameover ){
    window->draw(txtGameOver);
  }
  window->display();
}

void Tetris::run() {
    while (window->isOpen()) {
        // 统一处理所有输入事件
        events();
        
        // 根据状态绘制不同界面
        switch (currentState) {
            case GameState::MAIN_MENU:
                drawMainMenu();
                break;
                
            case GameState::INSTRUCTIONS:
                drawInstructions();
                break;
                
            case GameState::DIFFICULTY_SELECT:
                drawDifficultySelect();
                break;
                
            case GameState::PLAYING:
                // 游戏逻辑更新（包括自动下落）
                if (!gameover) {
                    float time = clock.getElapsedTime().asSeconds();
                    clock.restart();
                    timercount += time;

                    if (aiEnabled) simpleAI(); // 添加这行
                    
                    moveToDown();  // 这里处理自动下落
                    changePosition();
                    setRotate();
                    setScore();
                    resetValues();
                }
                draw();
                break;
                
            case GameState::GAME_OVER:
                draw();
                break;
        }
    }
}

void Tetris::moveToDown(){
  if( timercount > delay ){
    for (std::size_t i {}; i < squares; ++i) {
      k[i] = z[i];
      ++z[i].y; 
    }

    if( maxLimit() ){

      for (std::size_t i {}; i < squares; ++i) {
       area[ k[i].y ][ k[i].x ] = color; 
      }

      color = std::rand() % shapes + 1;
      std::uint32_t number = std::rand() % shapes;
      for (std::size_t i {}; i < squares; ++i) {
        z[i].x = forms[number][i] % 2; 
        z[i].y = forms[number][i] / 2; 
      }
    }

    timercount = 0;
  }

}

void Tetris::setRotate(){
  if( rotate ){
    Coords coords = z[1];
    for (std::size_t i {}; i < squares; ++i) {
      int x = z[i].y - coords.y; 
      int y = z[i].x - coords.x;

      z[i].x = coords.x - x;
      z[i].y = coords.y + y;
    }

    if( maxLimit() ){
      for (std::size_t i {}; i < squares; ++i) {
        z[i] = k[i]; 
      }
    }
  }
}

void Tetris::resetValues() {
    dirx = 0;
    rotate = false;
    // 移除 delay = 0.3f; 这行，改用基于难度的延迟
    switch(currentDifficulty) {
        case Difficulty::EASY:   delay = 0.5f; break;
        case Difficulty::MEDIUM: delay = 0.3f; break;
        case Difficulty::HARD:   delay = 0.1f; break;
    }
}

void Tetris::changePosition(){
  for (std::size_t i {}; i < squares; ++i) {
   k[i] = z[i];
   z[i].x += dirx; 
  }

  if( maxLimit() ){
    for (std::size_t i {}; i < squares; ++i) {
     z[i] = k[i]; 
    }
  }
}

bool Tetris::maxLimit() {
    for(std::size_t i{}; i < squares; ++i) {
        if(z[i].x >= cols || z[i].y >= lines ||  // 先检查边界
          (z[i].y >= 0 && z[i].x >= 0 && area[z[i].y][z[i].x])) { // 再检查碰撞
            return true;
        }
    }
    return false;
}

void Tetris::setScore(){
    std::uint32_t match = lines - 1;
    for (std::size_t i = match; i >= 1; --i) {
        std::uint32_t sum {};
        for (std::size_t j {}; j < cols; ++j) {
            if( area[i][j] ){
                if( i == 1 ){
                    gameover = true;
                }
                ++sum;
            }
            area[match][j] = area[i][j];
        } 
        if( sum < cols ){
            --match;
        }else{
            score++;
            // 每消3行获得一个炸弹
            if (score % 3 == 0) {
                bombs++;
                updateBombsDisplay();
            }
            txtScore.setString("SCORE: " + std::to_string(score));
        }
    }
}

void Tetris::useBomb() {
    if (bombs <= 0) return;

    // 1. 清除视觉最底部行（对应area[lines-1]）
    for (std::size_t j = 0; j < cols; ++j) {
        area[lines-1][j] = 0; // 清除方块
    }

    // 2. 所有方块下移一行（从顶部向底部移动）
    for (std::size_t i = lines-1; i > 0; --i) { // 注意倒序循环！
        for (std::size_t j = 0; j < cols; ++j) {
            area[i][j] = area[i-1][j]; // 当前行继承上一行的数据
        }
    }

    // 3. 确保视觉顶部行（area[0]）清零
    for (std::size_t j = 0; j < cols; ++j) {
        area[0][j] = 0;
    }

    // 4. 更新炸弹数量
    bombs--;
    updateBombsDisplay();
}

// 更新炸弹数量显示
void Tetris::updateBombsDisplay() {
    bombsText.setString("x" + std::to_string(bombs));
}

int Tetris::evaluateFlatness() const {
    // 记录每列当前高度
    std::array<int, cols> columnHeights = {0};
    
    // 1. 计算当前各列高度（从下往上扫描）
    for (int x = 0; x < cols; ++x) {
        for (int y = lines - 1; y >= 0; --y) {
            if (area[y][x] != 0) {
                columnHeights[x] = lines - y;
                break;
            }
        }
    }
    
    // 2. 添加当前方块后的高度变化
    for (int i = 0; i < squares; ++i) {
        if (z[i].x >= 0 && z[i].x < cols) {
            int newHeight = lines - z[i].y;
            if (newHeight > columnHeights[z[i].x]) {
                columnHeights[z[i].x] = newHeight;
            }
        }
    }
    
    // 3. 计算平整度（最高与最低列的差值，越小越好）
    int maxHeight = *std::max_element(columnHeights.begin(), columnHeights.end());
    int minHeight = *std::min_element(columnHeights.begin(), columnHeights.end());
    return -(maxHeight - minHeight); // 返回负值，差值越小得分越高
}

void Tetris::simpleAI() {
    if (!aiEnabled || gameover) return;

    // 保存原始状态
    Coords original[squares];
    std::copy(std::begin(z), std::end(z), std::begin(original));
    
    // 评估三个策略
    struct Option {
        int dirx;
        bool rotate;
        int score;
    };
    std::array<Option, 3> options = {
        Option{1, false, -999},  // 右移
        Option{-1, false, -999}, // 左移
        Option{0, true, -999}    // 旋转
    };
    
    // 测试每个选项
    for (auto& opt : options) {
        // 重置位置
        std::copy(std::begin(original), std::end(original), std::begin(z));
        
        // 应用操作
        if (opt.rotate) {
            rotate = true;
            setRotate();
            rotate = false;
        } else {
            dirx = opt.dirx;
            changePosition();
        }
        
        // 模拟硬降
        simulateHardDrop();
        
        // 计算得分 = 平整度 + 右侧倾向
        opt.score = evaluateFlatness() + (z[0].x - cols/2); // 右侧有轻微加分
    }
    
    // 选择最佳选项
    auto bestOpt = *std::max_element(options.begin(), options.end(),
        [](const Option& a, const Option& b) { return a.score < b.score; });
    
    // 执行最佳操作
    std::copy(std::begin(original), std::end(original), std::begin(z));
    if (bestOpt.rotate) {
        rotate = true;
        setRotate();
        rotate = false;
    } else {
        dirx = bestOpt.dirx;
        changePosition();
    }
    
    delay = 0.05f; // 加速下落
}

void Tetris::simulateHardDrop() {
    while (!maxLimit()) {
        for (int i = 0; i < squares; ++i) {
            k[i] = z[i];
            ++z[i].y;
        }
    }
    for (int i = 0; i < squares; ++i) {
        z[i] = k[i];
    }
}