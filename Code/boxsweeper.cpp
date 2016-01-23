#include <SFML/Graphics.hpp>

#include <iostream>
#include <time.h>
#include <stdio.h>

#define PADDING 15

const sf::Time TimePerFrame = sf::seconds(1.0f/60.0f);

enum alignment {LEFT, NONE, RIGHT};

enum state {playing, won, lost};
int gameState = playing;

int width = 1600;
int height = 900;

sf::Color hidden_tile_color = sf::Color(201, 200, 200, 255);
sf::Color revealed_tile_color = sf::Color(220, 220, 220, 255);
sf::Color tile_outline_color = sf::Color(255, 0, 0, 255);
sf::Color flag_color = sf::Color::Green;
sf::Color question_color = sf::Color::Magenta;
sf::Color bomb_color = sf::Color::Red;

sf::Vector2f prev;
sf::View view;
sf::View hud;

int bWidth;
int bHeight;
int bombs;

int totalTiles = bWidth * bHeight;
int clearTiles = totalTiles - bombs;
int bombFlags = 0;

int mark = 0;

bool setup = false;

struct Box
{
	sf::RectangleShape rect;
	bool Bomb;
	bool Flagged;
	bool Question;
	int Bordering;
	sf::Text text;
	bool Visible;
	Box()
	{
		Visible = false;
		Flagged = false;
		Question = false;
		Bomb = false;
		Bordering = 0;
		text.setString("0");
		rect.setFillColor(hidden_tile_color);
		rect.setOutlineColor(tile_outline_color);
		rect.setOutlineThickness(1);
	}
};
int counter = 0;
void InitVariables()
{
	totalTiles = bWidth * bHeight;
	clearTiles = totalTiles - bombs;
}

void RevealBlanks(Box** box, int i, int j)
{
	++counter;
	if (i < 0 || i >= bHeight)
		return;
	if (j < 0 || j >= bWidth)
		return;

	if (box[i][j].Visible || box[i][j].Flagged || box[i][j].Question)
		return;

	--clearTiles;
	box[i][j].Visible = true;
	if (box[i][j].Bordering != 0)
	{
		return;
	}

	if (i >= bWidth || j >= bHeight || i <= 0 || j <= 0)
		return;
	if (!box[i][j-1].Visible)
		RevealBlanks(box, i, j-1);
	if (!box[i-1][j-1].Visible)
		RevealBlanks(box, i-1, j-1);
	if (!box[i+1][j-1].Visible)
		RevealBlanks(box, i+1, j-1);
	if (!box[i][j+1].Visible)
		RevealBlanks(box, i, j+1);
	if (!box[i-1][j+1].Visible)
		RevealBlanks(box, i-1, j+1);
	if (!box[i+1][j+1].Visible)
		RevealBlanks(box, i+1, j+1);
	if (!box[i-1][j].Visible)
		RevealBlanks(box, i-1, j);
	if (!box[i+1][j].Visible)
		RevealBlanks(box, i+1, j);
}

void RevealBordering(Box** box, int i, int j)
{
	if (j-1 >= 0)
	{
		if (!box[i][j-1].Flagged)
		{
			if (box[i][j-1].Bordering == 0)
				RevealBlanks(box, i, j-1);
			box[i][j-1].Visible = true;
		}
		if (i-1 >= 0)
		{
			if (!box[i-1][j-1].Flagged)
			{
				if (box[i-1][j-1].Bordering == 0)
					RevealBlanks(box, i-1, j-1);
				box[i-1][j-1].Visible = true;
			}
		}
		if (i+1 < bHeight)
		{
			if (!box[i+1][j-1].Flagged)
			{
				if (box[i+1][j-1].Bordering == 0)
					RevealBlanks(box, i+1, j-1);
				box[i+1][j-1].Visible = true;
			}
		}
	}
	if (j+1 < bWidth)
	{
		if (!box[i][j+1].Flagged)
		{
			if (box[i][j+1].Bordering == 0)
				RevealBlanks(box, i, j+1);
			box[i][j+1].Visible = true;
		}
		if (i-1 >= 0)
		{
			if (!box[i-1][j+1].Flagged)
			{
				if (box[i-1][j+1].Bordering == 0)
					RevealBlanks(box, i-1, j+1);
				box[i-1][j+1].Visible = true;
			}
		}
		if (i+1 < bHeight)
		{
			if (!box[i+1][j+1].Flagged)
			{
				if (box[i+1][j+1].Bordering == 0)
					RevealBlanks(box, i+1, j+1);
				box[i+1][j+1].Visible = true;
			}
		}
	}
	if (i-1 >= 0)
	{
		if (!box[i-1][j].Flagged)
		{
			if (box[i-1][j].Bordering == 0)
				RevealBlanks(box, i-1, j);
			box[i-1][j].Visible = true;
		}
	}
	if (i+1 < bHeight)
	{
		if (!box[i+1][j].Flagged)
		{
			if (box[i+1][j].Bordering == 0)
				RevealBlanks(box, i+1, j);
			box[i+1][j].Visible = true;
		}
	}
}

void MiddleClick(Box** box, int i, int j)
{
	int found = 0;

	if (j-1 >= 0)
	{
		if (box[i][j-1].Flagged)
			++found;
		if (i-1 >= 0)
		{
			if (box[i-1][j-1].Flagged)
				++found;
		}
		if (i+1 < bHeight)
		{
			if (box[i+1][j-1].Flagged)
				++found;
		}
	}
	if (j+1 < bWidth)
	{
		if (box[i][j+1].Flagged)
			++found;
		if (i-1 >= 0)
		{
			if (box[i-1][j+1].Flagged)
				++found;
		}
		if (i+1 < bHeight)
		{
			if (box[i+1][j+1].Flagged)
				++found;
		}
	}
	if (i-1 >= 0)
	{
		if (box[i-1][j].Flagged)
			++found;
	}
	if (i+1 < bHeight)
	{
		if (box[i+1][j].Flagged)
			++found;
	}

	if (found == box[i][j].Bordering)
	{
		RevealBordering(box, i, j);
	}
	else
	{
		// Play sound here!
	}
}

void GenerateBoard(Box** box, int y, int x)
{
	int bombsPlaced = bombs;
	while (bombsPlaced > 0)
	{
		int xPos = rand() % bWidth;
		int yPos = rand() % bHeight;
		if (xPos >= x-2 && xPos <= x+2 && yPos >= y-2 && yPos <= y+2)
			continue;
		if (box[yPos][xPos].Bomb)
		{
			continue;
		}
		box[yPos][xPos].Bomb = true;
		box[yPos][xPos].Bordering = -1;
		--bombsPlaced;
	}

	for (int i = 0; i < bHeight; ++i)
	{
		for (int j = 0; j < bWidth; ++j)
		{
			if (box[i][j].Bomb)
			{
				box[i][j].text.setString("B");					// shows bombs
				box[i][j].text.setColor(sf::Color::Transparent);
				box[i][j].text.setColor(sf::Color::Cyan);			// shows bombs
			}
			//	(i, j)
			//	(0, 0)	(0, 1)	(0, 2)
			//	(1, 0)	(1, 1)	(1, 2)
			//	(2, 0)	(2, 1)	(2, 2)
			else if (!box[i][j].Bomb)
			{
				if (j-1 >= 0)
				{
					if (box[i][j-1].Bomb)
						++box[i][j].Bordering;
					if (i-1 >= 0)
					{
						if (box[i-1][j-1].Bomb)
							++box[i][j].Bordering;
					}
					if (i+1 < bHeight)
					{
						if (box[i+1][j-1].Bomb)
							++box[i][j].Bordering;
					}
				}
				if (j+1 < bWidth)
				{
					if (box[i][j+1].Bomb)
						++box[i][j].Bordering;
					if (i-1 >= 0)
					{
						if (box[i-1][j+1].Bomb)
							++box[i][j].Bordering;
					}
					if (i+1 < bHeight)
					{
						if (box[i+1][j+1].Bomb)
							++box[i][j].Bordering;
					}
				}
				if (i-1 >= 0)
				{
					if (box[i-1][j].Bomb)
						++box[i][j].Bordering;
				}
				if (i+1 < bHeight)
				{
					if (box[i+1][j].Bomb)
						++box[i][j].Bordering;
				}
				if (box[i][j].Bordering >= 0)
				{
					box[i][j].text.setString(std::to_string(box[i][j].Bordering));
					box[i][j].text.setColor(sf::Color::Transparent);
					if (box[i][j].Bordering != 0)
						box[i][j].text.setColor(sf::Color::Red);	// shows all number values
				}
			}
			//box[i][j].rect.setFillColor(sf::Color(200, 200, 200, 255));
		}
	}
}

bool Within(Box box, sf::Vector2f pos)
{
	bool result = false;
	if (pos.x >= box.rect.getPosition().x && pos.x <= box.rect.getPosition().x + box.rect.getSize().x &&
		pos.y >= box.rect.getPosition().y && pos.y <= box.rect.getPosition().y + box.rect.getSize().y)
	{
		result = true;
	}

	return result;
}

void InitText(sf::Text &text, sf::Font &font, int character_size, sf::Color color, std::string string, sf::Vector2f &pos, alignment align = NONE)
{
	text.setFont(font);
	text.setCharacterSize(character_size);
	text.setColor(color);
	text.setString(string);
	if (align == NONE)
		text.setPosition(pos.x, pos.y);
	else if (align == LEFT)
		text.setPosition(pos.x, pos.y - text.getLocalBounds().height);
	else if (align == RIGHT)
		text.setPosition(pos.x - text.getLocalBounds().width, pos.y - text.getLocalBounds().height);
}

void InitText(sf::Text &text, sf::Font &font, int character_size, sf::Color color, std::string string, float x, float y, alignment align = NONE)
{
	InitText(text, font, character_size, color, string, sf::Vector2f(x, y), align);
}

void InitRect(sf::RectangleShape &rect, float side_length, sf::Color color, float x, float y)
{
	rect.setSize(sf::Vector2f(side_length, side_length));
	rect.setFillColor(color);
	rect.setPosition(sf::Vector2f(x, y));
}

sf::Color LowerColor(sf::Color& color, int amount)
{
	sf::Uint8 r = 0;
	sf::Uint8 g = 0;
	sf::Uint8 b = 0;
	if (color.r >= amount)
		r = color.r - amount;
	if (color.g >= amount)
		g = color.g - amount;
	if (color.b >= amount)
		b = color.b - amount;
	return sf::Color(r, g, b, 255);
}

void LostGame(Box** board)
{
	for (int i = 0; i < bHeight; ++i)
	{
		for (int j = 0; j < bWidth; ++j)
		{
			if (!board[i][j].Visible)
			{
				sf::Color temp;
				temp = LowerColor(hidden_tile_color, 40);
				board[i][j].rect.setFillColor(temp);
				
				if (board[i][j].Bordering != 0 && !board[i][j].Bomb)
				{
					board[i][j].text.setColor(sf::Color::Red);
				}
				else if (board[i][j].Bomb)
				{
					temp = LowerColor(bomb_color, 40);
					board[i][j].rect.setFillColor(temp);
					board[i][j].text.setColor(sf::Color::Transparent);
				}
			}
		}
	}
}

void PollEvents(sf::RenderWindow& window, Box** board)
{
	sf::Vector2i pixel_pos;
	sf::Vector2f world_pos;
	sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			std::cout << "Closed" << std::endl;
			window.close();
		}
		if(event.type == sf::Event::Resized)
		{
			window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
			view.setCenter(view.getSize() * 0.5f);
		}
		if (event.type == sf::Event::MouseMoved)
		{
			sf::Vector2f ms;
			sf::Vector2f delta;
			ms = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
			delta = ms - prev;
			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
			{
				//view.setCenter(view.getCenter() - delta);
				view.move(-delta);
			}
			prev = ms;
		}
		if (event.type == sf::Event::MouseWheelMoved)
		{
			if (event.mouseWheel.delta > 0)
			{
				view.setSize(view.getSize().x * 0.75, view.getSize().y * 0.75);
				view.zoom(0.75f);
			}
			else
			{
				view.setSize(view.getSize().x * 1.25, view.getSize().y * 1.25);
				view.zoom(1.25f);
			}
		}
		if (event.type == sf::Event::MouseButtonPressed && event.key.code == sf::Mouse::Left)
		{
			if (gameState == playing)
			{
				pixel_pos = sf::Mouse::getPosition(window);
				world_pos = window.mapPixelToCoords(pixel_pos);
				int i;
				int j;
				for (i = 0; i < bHeight; ++i)
				{
					for (j = 0; j < bWidth; ++j)
					{
						if (Within(board[i][j], world_pos))
						{
							if (!setup)
							{
								setup = true;
								GenerateBoard(board, i, j);
							}
							if (!board[i][j].Flagged && !board[i][j].Question)
							{
								if (board[i][j].Bordering == 0)
									RevealBlanks(board, i, j);
								else
								{
									board[i][j].Visible = true;
								}
								printf("Tiles revealed: %i, percent of board: %f\n", counter, (float)counter/totalTiles*100);
							}
						}
					}
				}
			}
		}
		if (event.type == sf::Event::MouseButtonPressed && event.key.code == sf::Mouse::Right)
		{
			if (gameState == playing)
			{
				pixel_pos = sf::Mouse::getPosition(window);
				world_pos = window.mapPixelToCoords(pixel_pos);
				for (int i = 0; i < bHeight; ++i)
				{
					for (int j = 0; j < bWidth; ++j)
					{
						if (Within(board[i][j], world_pos))
						{
							if (!board[i][j].Visible)
							{
								if (!board[i][j].Flagged && !board[i][j].Question)
								{
									board[i][j].rect.setFillColor(flag_color);
									board[i][j].Flagged = true;
									++bombFlags;
								}
								else if (board[i][j].Flagged)
								{
									board[i][j].rect.setFillColor(question_color);
									board[i][j].Flagged = false;
									board[i][j].Question = true;
									--bombFlags;
								}
								else if (board[i][j].Question)
								{
									board[i][j].rect.setFillColor(hidden_tile_color);
									board[i][j].Question = false;
								}
							}
						}
					}
				}
			}
		}
		if (event.type == sf::Event::MouseButtonPressed && event.key.code == sf::Mouse::Middle)
		{
			pixel_pos = sf::Mouse::getPosition(window);
			world_pos = window.mapPixelToCoords(pixel_pos);
			for (int i = 0; i < bHeight; ++i)
			{
				for (int j = 0; j < bWidth; ++j)
				{
					if (Within(board[i][j], world_pos))
					{
						if (board[i][j].Visible)
							MiddleClick(board, i, j);
					}
				}
			}
		}
	}
}

int main()
{
	int in_width;
	int in_height;
	int in_bombs;
	int choice;
	do {printf("Enter 1 for options, or 2 to use existing settings: ");
		scanf("%i", &choice);
	} while (choice < 1 || choice > 2);
	if (choice == 1)
	{
		do {printf("Enter board width from 1 - 80: ");
			scanf("%i", &in_width);
		} while (in_width < 1 || in_width > 15000);
		do {printf("Enter board height from 1 - 68: ");
			scanf("%i", &in_height);
		} while (in_height < 1 || in_height > 15000);
		do {printf("Enter number of bombs from 1 to %i: ", (in_width*in_height)/2);
			scanf("%i", &in_bombs);
		} while (in_bombs < 1 || in_bombs > (in_width*in_height)/2);

		FILE* ofp = fopen("config.txt", "w");
		fprintf(ofp, "%i ", in_width);
		fprintf(ofp, "%i ", in_height);
		fprintf(ofp, "%i ", in_bombs);
		fclose(ofp);
	}

	FILE* ifp = fopen("config.txt", "r");
	fscanf(ifp, "%i", &bWidth);
	fscanf(ifp, "%i", &bHeight);
	fscanf(ifp, "%i", &bombs);
	InitVariables();

	printf("Total tiles: %i\n", totalTiles);

	srand(time(NULL));
	
	sf::RenderWindow window(sf::VideoMode(width, height, 32), "Boxsweeper", sf::Style::Default);
	view.reset(sf::FloatRect(0, 0, width, height));
	view.setViewport(sf::FloatRect(0, 0, 1, 0.95));
	hud.reset(sf::FloatRect(0, 0, width, height));
	hud.setViewport(sf::FloatRect(0, 0.95, 1, 1));
	sf::Font font;
	if (!font.loadFromFile("Arial.ttf"))
	{
	}

	sf::Text win;
	InitText(win, font, 26, sf::Color::Blue, "You won!", width - PADDING, PADDING, RIGHT);

	sf::Text lose;
	InitText(lose, font, 26, sf::Color::Red, "You lost!", width - PADDING, PADDING, RIGHT);

	sf::Text wrong;
	InitText(wrong, font, 26, sf::Color::Green, "Something's not quite right... Check your flags again", width - PADDING, PADDING, RIGHT);

	sf::Text flag;
	InitText(flag, font, 26, sf::Color::Green, "Flag", 0, 0, NONE);

	sf::RectangleShape flagRect;
	InitRect(flagRect, 20.0f, sf::Color::Green, flag.getPosition().x + flag.getLocalBounds().width + PADDING, flag.getPosition().y + flag.getLocalBounds().height/2);

	sf::Text question;
	InitText(question, font, 26, sf::Color::Magenta, "Question", flagRect.getPosition().x + flagRect.getLocalBounds().width + PADDING, 0, NONE);

	sf::RectangleShape questionRect;
	InitRect(questionRect, 20.0f, sf::Color::Magenta, question.getPosition().x + question.getLocalBounds().width + PADDING, question.getPosition().y + question.getLocalBounds().height/2);

	sf::Text left;
	InitText(left, font, 26, sf::Color::Red, "Bombs Left: " + std::to_string(bombs), questionRect.getPosition().x + questionRect.getLocalBounds().width + PADDING, 0, NONE);

	sf::Text time;
	InitText(time, font, 26, sf::Color::Red, "Time: 00", left.getPosition().x + left.getLocalBounds().width + PADDING, 0, NONE);

	Box** board = new Box*[bHeight];
	for (int k = 0; k < bHeight; ++k)
		board[k] = new Box[bWidth];

	for (int i = 0; i < bHeight; ++i)
	{
		for (int j = 0; j < bWidth; ++j)
		{
			if ((height-50)/bHeight < 30)
				board[i][j].rect.setSize(sf::Vector2f(30.0f, 30.0f));
			else
				board[i][j].rect.setSize(sf::Vector2f((float)(height-50)/bHeight, (float)(height-50)/bHeight));
			board[i][j].rect.setPosition(5 + (j * board[i][j].rect.getSize().x), 5 + (i * board[i][j].rect.getSize().y));
			board[i][j].text.setFont(font);
			board[i][j].text.setCharacterSize((unsigned int)(board[i][j].rect.getSize().y - 4));
			board[i][j].text.setPosition(board[i][j].rect.getPosition().x + board[i][j].text.getLocalBounds().width/2, board[i][j].rect.getPosition().y);
			board[i][j].text.setString("");
		}
	}

	sf::Clock clock;
	sf::Clock clock2;
	sf::Time timeSinceLastUpdate = sf::Time::Zero;
	sf::Time timer = sf::Time::Zero;

	while (window.isOpen())
	{
		timeSinceLastUpdate += clock.restart();
		timer = clock2.getElapsedTime();
		while (timeSinceLastUpdate > TimePerFrame)
		{
			timeSinceLastUpdate -= TimePerFrame;
			PollEvents(window, board);

			left.setString("Bombs Left: " + std::to_string(bombs-bombFlags));

			if (bombFlags == bombs)
			{
				bool ok = true;
				for (int i = 0; i < bHeight; ++i)
				{
					for (int j = 0; j < bWidth; ++j)
					{
						if (board[i][j].Bomb && !board[i][j].Flagged)
						{
							wrong.setColor(sf::Color::Green);
							ok = false;
						}
					}
				}
				if (ok)
				{
					gameState = won;
				}
			}
			else
			{
				wrong.setColor(sf::Color::Transparent);
			}

			if (gameState == playing)
			{
				if (timer.asSeconds() > mark)
				{
					time.setString("Time: " + std::to_string(mark));
					time.setPosition(left.getPosition().x + left.getLocalBounds().width + PADDING, 0);
				}

				mark = timer.asSeconds();
			}

			for (int i = 0; i < bHeight; ++i)
			{
				for (int j = 0; j < bWidth; ++j)
				{
					if (board[i][j].Visible)
					{
						if (board[i][j].Bomb)
						{
							board[i][j].rect.setFillColor(sf::Color::Red);
							gameState = lost;
						}
						else if (board[i][j].rect.getFillColor() == hidden_tile_color)
						{
							board[i][j].rect.setFillColor(revealed_tile_color);
							if (board[i][j].Bordering != 0)
							{
								board[i][j].text.setColor(sf::Color::Red);
							}
						}
					}
				}
			}

			if (gameState == lost)
			{
				LostGame(board);
			}
		}

		window.setView(view);
		window.clear(sf::Color(255, 255, 255));
		for (int i = 0; i < bHeight; ++i)
		{
			for (int j = 0; j < bWidth; ++j)
			{
				window.draw(board[i][j].rect);
				window.draw(board[i][j].text);
			}
		}
		window.setView(hud);
		if (gameState == won)
		{
			window.draw(win);
		}
		else if (gameState == lost)
		{
			window.draw(lose);
		}
		window.draw(wrong);
		window.draw(flag);
		window.draw(flagRect);
		window.draw(question);
		window.draw(questionRect);
		window.draw(left);
		window.draw(time);
		window.setView(view);
		window.display();
	}

	return 0;
}