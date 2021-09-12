#ifndef __LAVASTONE_GENDATA_HPP__
#define __LAVASTONE_GENDATA_HPP__

#include <stdlib.h>
#include <string>
#include <vector>

// static std::vector<std::string> adjectives, nouns, titles, names, directions,
// cities, states;
static const std::vector<std::string> adjectives = {
    "Delicious", "Green",   "Beautiful", "Fresh",  "Clear", "Smelly",
    "Fragrant",  "Salty",   "Sweet",     "Sour",   "Spicy", "Funky",
    "Chunky",    "Hot",     "Cold",      "Thick",  "Thin",  "Baked",
    "Fried",     "Onion",   "Fruity",    "Greasy", "Raw",   "Delightful",
    "Aged",      "Pumpkin", "Crunchy"};
static const std::vector<std::string> nouns = {
    "Bread",          "Chunks",  "Pudding", "Fondue", "Fudge",
    "Salad",          "Pancake", "Sausage", "Beans",  "Pork",
    "Liver",          "Brittle", "Bombé",   "Jelly",  "Salad",
    "Bluegill",       "Platter", "Punch",   "Loaf",   "Breadsticks",
    "Mushroom Sauté", "Filet",   "Cobb",    "Chow",   "Surprise"};
static const std::vector<std::string> titles = {
    "Grandpapa",  "Grandmama",   "Professor",   "Granddaughter", "Grandmaster",
    "Grandson",   "Grandniece",  "Grandnephew", "Uncle",         "Auntie",
    "Great-Aunt", "Great Uncle", "Farmer",      "Father",        "Mother",
    "Daddy",      "Mommy",       "Captain"};
static const std::vector<std::string> names = {
    "Jocelyn",  "Josephine", "Jessica",  "Jeborah", "Jordan",  "Jerald",
    "Jeronica", "Jeremiah",  "Jedediah", "Johann",  "Jemimah", "J-Lo"};
static const std::vector<std::string> directions = {
    "North",      "South",           "East",  "West",
    "NorthSouth", "West-NorthSouth", "Upper", "Lower"};
static const std::vector<std::string> cities = {
    "Townesville", "Statesville", "Townington", "Humansburg",
    "Peoplestown", "Realville",   "Realtown",   "Sun Funcisco"};
static const std::vector<std::string> states = {"Statesylvania", "Statifornia",
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
bool operator==(const recipe &lhs, const recipe &rhs);
std::ostream &operator<<(std::ostream &os, const recipe &r);

recipe random_recipe();
std::vector<recipe> random_recipes(size_t n);

#endif
