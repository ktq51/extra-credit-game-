#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <cstdlib>
#include <ctime>
using namespace std;

struct Item {
    string name;
    string type; 
    int power;
};

struct Quest {
    string name;
    string description;
    bool completed;
};

class Character {
public:
    string name;
    string className;
    int maxHealth, health, mp, strength, defense, xp;
    vector<Item> inventory;

    Character(string n, string c) : name(n), className(c), xp(0) {
        if (c == "Warrior") {
            maxHealth = 120; mp = 20; strength = 20; defense = 10;
        } else if (c == "Mage") {
            maxHealth = 80; mp = 50; strength = 25; defense = 5;
        } else {
            maxHealth = 100; mp = 30; strength = 15; defense = 8;
        }
        health = maxHealth;
    }

    Character(string n, int hp, int atk, int def) : name(n), className("Enemy"), xp(0),
        maxHealth(hp), health(hp), mp(0), strength(atk), defense(def) {}

    bool isAlive() const { return health > 0; }

    void takeDamage(int dmg) {
        int damage = max(0, dmg - defense);
        health = max(0, health - damage);
        cout << name << " takes " << damage << " damage. Health: " << health << "\n";
    }

    void heal(int amount) {
        health = min(maxHealth, health + amount);
        cout << name << " heals for " << amount << ". Health: " << health << "\n";
    }

    void useItem(const string& itemName) {
        for (auto it = inventory.begin(); it != inventory.end(); ++it) {
            if (it->name == itemName) {
                if (it->type == "potion") {
                    heal(it->power);
                }
                inventory.erase(it);
                return;
            }
        }
        cout << "Item not found in inventory.\n";
    }

    void showInventory() {
        cout << "--- Inventory ---\n";
        for (const auto& item : inventory) {
            cout << item.name << " (" << item.type << ", Power: " << item.power << ")\n";
        }
        if (inventory.empty()) cout << "Inventory is empty.\n";
    }

    void saveToFile() {
        ofstream out("save.txt");
        out << name << '\n' << className << '\n'
            << health << '\n' << mp << '\n' << strength << '\n'
            << defense << '\n' << xp << '\n' << maxHealth << '\n';
        out << inventory.size() << '\n';
        for (const auto& item : inventory)
            out << item.name << '\n' << item.type << '\n' << item.power << '\n';
        out.close();
    }

    void loadFromFile() {
        ifstream in("save.txt");
        if (!in) { cout << "No save found.\n"; return; }
        int invSize;
        in >> ws;
        getline(in, name);
        getline(in, className);
        in >> health >> mp >> strength >> defense >> xp >> maxHealth >> invSize;
        inventory.clear();
        for (int i = 0; i < invSize; ++i) {
            Item it;
            in >> ws;
            getline(in, it.name);
            getline(in, it.type);
            in >> it.power;
            inventory.push_back(it);
        }
        in.close();
    }
};

struct Location {
    string description;
    string enemyName;
    int enemyHealth, enemyAttack, enemyDefense;
    bool hasEnemy, visited, isShop;
};

map<pair<int, int>, Location> gameMap;
vector<Quest> quests;
Character* player = nullptr;
int x = 0, y = 0;

void battle(Character& p, const Character& originalEnemy) {
    Character enemy = originalEnemy;
    cout << "\n-- Battle: " << p.name << " vs " << enemy.name << " --\n";

    while (p.isAlive() && enemy.isAlive()) {
        string action;
        cout << "\nChoose action: attack / skill / item / flee\n";
        cin >> action;

        if (action == "attack") {
            enemy.takeDamage(p.strength + rand() % 5);
        } else if (action == "skill") {
            if (p.mp >= 10) {
                p.mp -= 10;
                enemy.takeDamage(p.strength + 10 + rand() % 10);
                cout << "You use a powerful skill! MP left: " << p.mp << "\n";
            } else {
                cout << "Not enough MP!\n";
            }
        } else if (action == "item") {
            p.showInventory();
            cout << "Enter item name: ";
            cin.ignore();
            string itemName;
            getline(cin, itemName);
            p.useItem(itemName);
        } else if (action == "flee") {
            if (rand() % 2 == 0) {
                cout << "You escaped!\n";
                return;
            } else {
                cout << "Failed to flee!\n";
            }
        }

        if (enemy.isAlive()) {
            p.takeDamage(enemy.strength + rand() % 6);
        }
    }

    if (p.isAlive()) {
        cout << enemy.name << " defeated! You gain 10 XP.\n";
        p.xp += 10;
    } else {
        cout << "You died. Game Over.\n";
        exit(0);
    }
}

void showMap() {
    cout << "\n--- Map ---\n";
    for (int j = 4; j >= 0; --j) {
        for (int i = 0; i <= 4; ++i) {
            auto loc = gameMap[{i, j}];
            char symbol = loc.hasEnemy ? 'M' : (loc.isShop ? 'P' : (loc.visited ? 'E' : 'N'));
            if (i == x && j == y) cout << "[" << symbol << "]";
            else cout << " " << symbol << " ";
        }
        cout << "\n";
    }
}

void showQuests() {
    cout << "\n--- Quests ---\n";
    for (auto& q : quests) {
        cout << q.name << ": " << q.description;
        cout << (q.completed ? " (Completed)\n" : " (In Progress)\n");
    }
}

void explore() {
    auto& loc = gameMap[{x, y}];
    if (!loc.visited) {
        cout << "\nYou discover " << loc.description << ".\n";
        loc.visited = true;

        if (loc.isShop) {
            cout << "A cat vendor heals you and gives you a potion!\n";
            player->heal(player->maxHealth);
            player->inventory.push_back({"Healing Tuna", "potion", 30});
        }

        if (loc.hasEnemy) {
            Character enemy(loc.enemyName, loc.enemyHealth, loc.enemyAttack, loc.enemyDefense);
            battle(*player, enemy);
            loc.hasEnemy = false;
            if (enemy.name == "Lord Purrmort") {
                cout << "You saved the Catdom! Victory!\n";
                exit(0);
            }
        }
    } else {
        cout << "You are at " << loc.description << ".\n";
    }
    showMap();
}

void createCharacter() {
    string name, cls;
    cout << "Enter your name: ";
    cin >> ws;
    getline(cin, name);
    cout << "Choose a class (Warrior / Mage / Rogue): ";
    cin >> cls;
    player = new Character(name, cls);
    cout << "Welcome, " << player->name << " the " << player->className << "!\n";
}

void setupMap() {

    gameMap[{0, 0}] = {"Whisker Valley", "", 0, 0, 0, false, false, false};
    gameMap[{1, 0}] = {"Silent Cliffs", "Mewsassin", 50, 12, 4, true, false, false};
    gameMap[{2, 0}] = {"Starlight Clearing", "", 0, 0, 0, false, false, false};
    gameMap[{3, 0}] = {"Fish Market", "", 0, 0, 0, false, false, true};
    gameMap[{4, 0}] = {"Cozy Den", "", 0, 0, 0, false, false, false};

    gameMap[{0, 1}] = {"Lone Rock", "", 0, 0, 0, false, false, false};
    gameMap[{1, 1}] = {"Shadow Marsh", "Sir Shredsalot", 60, 14, 6, true, false, false};
    gameMap[{2, 1}] = {"Mossy Crossing", "", 0, 0, 0, false, false, false};
    gameMap[{3, 1}] = {"Potion Pond", "", 0, 0, 0, false, false, true};
    gameMap[{4, 1}] = {"Purr Path", "", 0, 0, 0, false, false, false};

    gameMap[{0, 2}] = {"Wispwood Edge", "", 0, 0, 0, false, false, false};
    gameMap[{1, 2}] = {"Dryroot Hollow", "", 0, 0, 0, false, false, false};
    gameMap[{2, 2}] = {"Meowtain Pass", "", 0, 0, 0, false, false, false};
    gameMap[{3, 2}] = {"Catnip Fields", "", 0, 0, 0, false, false, false};
    gameMap[{4, 2}] = {"The Purring Path", "", 0, 0, 0, false, false, false};

    gameMap[{0, 3}] = {"Thicket Trail", "", 0, 0, 0, false, false, false};
    gameMap[{1, 3}] = {"Dark Forest with Meowgar", "Meowgar the Mad", 60, 15, 5, true, false, false};
    gameMap[{2, 3}] = {"Echo Ridge", "", 0, 0, 0, false, false, false};
    gameMap[{3, 3}] = {"Bark Plains", "", 0, 0, 0, false, false, false};
    gameMap[{4, 3}] = {"Twilight Meadow", "", 0, 0, 0, false, false, false};

    gameMap[{0, 4}] = {"Whispering Woods", "", 0, 0, 0, false, false, false};
    gameMap[{1, 4}] = {"Moonlight Grove", "", 0, 0, 0, false, false, false};
    gameMap[{2, 4}] = {"Throne Room of Lord Purrmort", "Lord Purrmort", 100, 20, 10, true, false, false};
    gameMap[{3, 4}] = {"Temple of Yarn", "", 0, 0, 0, false, false, false};
    gameMap[{4, 4}] = {"Crystal Hill", "", 0, 0, 0, false, false, false};

    x = 0;
    y = 0;
}


void setupQuests() {
    quests.push_back({"Defeat Meowgar", "Find and defeat Meowgar the Mad.", false});
    quests.push_back({"Collect Barkshield", "Defeat Sir Shredsalot and collect the Barkshield.", false});
}

int main() {
    srand(time(0));
    setupMap();
    setupQuests();

    cout << "Feline Fantasy: The Cat Chronicles\n";
    cout << "1. New Game\n2. Load Game\n3. Quit\n> ";
    int choice;
    cin >> choice;

    if (choice == 1) {
        createCharacter();
    } else if (choice == 2) {
        player = new Character("", "");
        player->loadFromFile();
        cout << "Welcome back, " << player->name << "!\n";
    } else {
        return 0;
    }

    while (true) {
        explore();
        cout << "\nCommands: north / south / east / west / save / quests / inv / quit\n> ";
        string cmd;
        cin >> cmd;
        int newX = x, newY = y;

        if (cmd == "north") newY++;
        else if (cmd == "south") newY--;
        else if (cmd == "east") newX++;
        else if (cmd == "west") newX--;
        else if (cmd == "save") player->saveToFile();
        else if (cmd == "quests") showQuests();
        else if (cmd == "inv") player->showInventory();
        else if (cmd == "quit") break;
        else continue;

        if (gameMap.find({newX, newY}) != gameMap.end()) {
            x = newX; y = newY;
        } else {
            cout << "You can't go that way.\n";
        }
    }

    return 0;
}



