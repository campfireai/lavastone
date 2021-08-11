#include "gendata.hpp"
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

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
