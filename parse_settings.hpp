#ifndef PARSE_SETTINGS_HPP
#define PARSE_SETTINGS_HPP
#include <boost/foreach.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <exception>
#include <iostream>
#include <set>
#include <string>

void loadConfig(std::string const &filename)
{
  boost::property_tree::ptree tree;
  boost::property_tree::read_json(filename, tree);
}
#endif
