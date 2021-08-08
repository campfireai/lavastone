#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

std::vector<std::string> adjectives = {
    "Delicious", "Green", "Beautiful",  "Fresh", "Clear",  "Smelly", "Fragrant",
    "Salty",     "Sweet", "Sour",       "Spicy", "Funky",  "Chunky", "Hot",
    "Cold",      "Thick", "Thin",       "Baked", "Fried",  "Onion",  "Fruity",
    "Greasy",    "Raw",   "Delightful", "Aged",  "Pumpkin"};
std::vector<std::string> nouns = {
    "Bread",          "Chunks",  "Pudding", "Fondue", "Fudge",
    "Salad",          "Pancake", "Sausage", "Beans",  "Pork",
    "Liver",          "Brittle", "Bombé",   "Jelly",  "Salad",
    "Bluegill",       "Platter", "Punch",   "Loaf",   "Breadsticks",
    "Mushroom Sauté", "Filet",   "Cobb"};
std::vector<std::string> titles = {
    "Grandpapa",  "Grandmama",   "Professor",   "Granddaughter", "Grandmaster",
    "Grandson",   "Grandniece",  "Grandnephew", "Uncle",         "Auntie",
    "Great-Aunt", "Great Uncle", "Farmer",      "Father",        "Mother",
    "Daddy",      "Mommy",       "Captain"};
std::vector<std::string> names = {
    "Jocelyn",  "Josephine", "Jessica",  "Jeborah", "Jordan",  "Jerald",
    "Jeronica", "Jeremiah",  "Jedediah", "Johann",  "Jemimah", "J-Lo"};
std::vector<std::string> directions = {"North", "South",      "East",
                                       "West",  "NorthSouth", "West-NorthSouth",
                                       "Upper", "Lower"};
std::vector<std::string> cities = {"Townesville", "Statesville", "Townington",
                                   "Humansburg",  "Peoplestown", "Realville"};
std::vector<std::string> states = {"Statesylvania", "Statifornia",
                                   "Stately State", "Statington",
                                   "Statesachusetts"};

template <typename T> T random_element(const std::vector<T> &vec) {
  return vec.at(std::rand() % vec.size());
}

struct recipe {
  std::string title;
  std::string author;
  std::string author_location;
  int num_likes;
};

recipe random_recipe() {
  recipe r;
  r.author = random_element(titles) + " " + random_element(names);
  r.title = r.author + "'s " + random_element(adjectives) + " " +
            random_element(nouns);
  r.author_location = random_element(directions) + " " +
                      random_element(cities) + ", " + random_element(states);
  r.num_likes = std::rand() % 10000;
  return r;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "usage:\n"
              << "./gendata NUM_RECORDS\n";
    exit(1);
  }

  size_t num_records = std::stoi(argv[1]);

  std::ofstream outfile;
  outfile.open("data.csv");
  outfile << "title,author,author_location"
          << "\n";
  for (size_t i = 0; i < num_records; i++) {
    recipe r = random_recipe();
    outfile << "\"" << r.title << "\",\"" << r.author << "\",\""
            << r.author_location << "\"\n";
  }
  outfile.close();
}
