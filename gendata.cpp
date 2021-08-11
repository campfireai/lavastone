#include "gendata.hpp"
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

bool operator==(const recipe &lhs, const recipe &rhs) {
  return lhs.title == rhs.title && lhs.author == rhs.author &&
         lhs.author_location == rhs.author_location &&
         lhs.num_likes == rhs.num_likes;
}
std::ostream &operator<<(std::ostream &os, const recipe &r) {

  os << "\033[35m"
     << "Recipe:\n\033[33m\"" << r.title << "\"\n\033[39m"
     << "submitted by: \033[36m" << r.author << "\033[39m, \033[34m"
     << r.author_location << "\n\033[32m" << r.num_likes << " Likes\033[39m";
  return os;
}

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

std::vector<recipe> random_recipes(size_t n) {
  std::vector<recipe> recipes;
  recipes.reserve(n);

  for (size_t i = 0; i < n; i++) {
    recipes.push_back(random_recipe());
  }

  return recipes;
}
