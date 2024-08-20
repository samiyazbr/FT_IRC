/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hkunnam- <hkunnam-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:45:16 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/02/11 11:40:08 by hkunnam-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <vector>
#include <string>
#include <iostream>

// USED IN convert_to_vector FUNCTION
//  checks if the char c exists within string str
// 1. returns ture if the character is found else will return false
bool is_inside(const std::string & str, char c)
{
    return str.find(c) != std::string::npos;
}

// USED IN invite, join, kick, mode, notice, part, pong, privmsg, topic FUNCTIONS
// takes a string s and splits it into tokens based on whitespace characters.
// 1. iterates through the charecters in the string and construct tokens by
//	  skipping whitespace chars and collecting consecutive non-whitespace chars.
// 2. the tokens are then stored in a vector of strings.
std::vector<std::string> convert_to_vector(std::string s)
{
    std::vector<std::string> tokens;
    std::string token = "\0";
    std::string str = " \t\n\r\f\v";
    
    for (size_t i = 0; i < s.size(); i++)
    {
        while (is_inside(str, s[i]) && i < s.size())
            i++;
        while (!is_inside(str, s[i]) && i < s.size())
        {
            token += s[i];
            i++;
        }
        if (is_inside(str, s[i]) && i < s.size())
        {
            tokens.push_back(token);
            token.clear();
        }
    }
    if (token != "\0")
        tokens.push_back(token);
    return tokens;
}

// USED IN TESTING
// prints the elements of a vector of strings
// 1. it prints the size of the vector
// 2. iterates through the vector and prints each element preceeded by a (-)
void print_vector(std::vector<std::string> vec)
{
    std::vector<std::string>::iterator it;

    std::cout << "+++++++++++++\n";
    std::cout << "vector size is" << vec.size() << " \n";
    for (it = vec.begin(); it != vec.end(); it++)
        std::cout << "-" << *it << "\n";
    std::cout << "+++++++++++++\n";
}

// USED IN INVITE, JOIN, KICK, PART, TOPIC 
// splits a string based on a delimiter charecter
// 1. iterates through the charecters of the string and pushes the chars 
// into a vector untill the delimiter is found.
// 2. when the delimiter is found the current token is pushed back into the vector
// and a new token is started.
// 3. returns the token.
std::vector<std::string> split(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    for (size_t i = 0; i < s.size(); i++)
    {
        if (s[i] == delimiter)
        {
            tokens.push_back(token);
            token = "";
        }
        else
        {
            token += s[i];
        }
    }
    tokens.push_back(token);
    return tokens;
}

// USED IN JOIN
// splits and converts the chars into lowercase and pushes them to vector.
std::vector<std::string> splitlower(const std::string &s, char delimiter) //split and store as lower case
{
    std::vector<std::string> tokens;
    std::string token;
    for (size_t i = 0; i < s.size(); i++)
    {
        if (s[i] == delimiter)
        {
            tokens.push_back(token);
            token = "";
        }
        else
        {
            token += tolower(s[i]);
        }
    }
    tokens.push_back(token);
    return tokens;
}

/*	USED IN KICK, NOTICE, PART, PRVMSG 
joins the elements of a vector into a single string starting from
the second element.
1.	iterates through the vector starting from the second element.
2.	adds each element followed by a space to a single string.
3.	returns the string. */
std::string ParamsJoin(std::vector<std::string> vec)
{
    std::vector<std::string>::iterator it;
    std::string text;
    
    for (it = vec.begin() + 1; it != vec.end(); it++)
    {
        text += (*it + " ");
    }
    return text;
}

/* USED IN PARSING 
Trims leading whitespace characters from a string
1.	looks for the index of the first char that is not in the specified whitespace chars
2.	erases all chars before the index.
3.	returns modified string */
std::string& lefttrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

/* USED IN PARSING
Trims trailing whitespace charecters from a string
1. looks for the index of the last char that is not in the specified white space chars
2.	erases all chars after that index.
3. returns modified string */
std::string& righttrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}