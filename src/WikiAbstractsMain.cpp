// Copyright 2019, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Patrick Brosi <brosi@informatik.uni-freiburg.de>

#include <iostream>
#include "pfxml.h"

enum class RetCode { SUCCESS = 0 };

enum TextStage { LBEG,  TEXT, IN_CURL, IN_SQ, IN_H, IN_H_TIT, IN_H_CL};

// _____________________________________________________________________________
void printAbstract(const char* text) {
  size_t pos = 0;

  TextStage s = LBEG;
  size_t HEAD_D = 0;
  size_t CURL_D = 0;
  size_t BOX_D = 0;

  size_t HEAD_BEG = 0;

  std::string tmp;

  while (text[pos]) {
    switch (s) {
      case LBEG:
        if (std::isspace(text[pos])) {
          s = TEXT;
          continue;
        } else if (text[pos] == '=') {
          s = IN_H;
          HEAD_D += 1;
          HEAD_BEG = pos;
          pos++;
          continue;
        } else if (text[pos] == '*') {
          // ignore line
          // pos = strchr(text + pos, '\n');
        }
        s = TEXT;
        continue;

      case IN_H:
        if (std::isspace(text[pos])) {
          pos++;
          continue;
        } else if (text[pos] == '=') {
          HEAD_D += 1;
          pos++;
          continue;
        }

        s = IN_H_TIT;
        pos++;
        continue;

      case IN_H_TIT:
        if (text[pos] == '=') {
          HEAD_D -= 1;
          s = IN_H_CL;
          pos++;
          continue;
        }

        pos++;
        continue;

      case IN_H_CL:
        if (text[pos] == '=') {
          HEAD_D -= 1;
          if (HEAD_D == 0) {
            return;
          }
          pos++;
          continue;
        } else {
          s = TEXT;
          continue;
        }

      case IN_CURL:
        if (text[pos] == '}' && text[pos+1] == '}') {
          CURL_D -= 1;
          if (CURL_D == 0) {
            s = TEXT;
          }

          pos += 2;
          continue;

        } else if (text[pos] == '{' && text[pos+1] == '{') {
          CURL_D += 1;
          pos += 2;
          continue;
        }

        pos++;
        continue;

      case IN_SQ:
        if (text[pos] == '|' || text[pos] == ':') {
          tmp.clear();
          pos++;
          continue;
        } else if (text[pos] == ']' && text[pos+1] == ']') {
          std::cout << tmp;
          pos += 2;
          s = TEXT;
          continue;
        }
        tmp += text[pos];
        pos++;
        continue;


      case TEXT:
        if (text[pos] == '\n') {
          std::cout << ' ';
          pos++;
          s = LBEG;
          continue;
        } else {
          if (text[pos] == '{' && text[pos+1] == '{') {
            s = IN_CURL;
            CURL_D = 1;
            pos += 2;
            continue;
          } else if (text[pos] == '[' && text[pos+1] == '[') {
            s = IN_SQ;
            tmp.clear();
            pos += 2;
            continue;
          }
        }
        std::cout << text[pos];
        pos++;
        continue;
    }
  }
}

// _____________________________________________________________________________
int main(int argc, char** argv) {
  // disable output buffering for standard output
  setbuf(stdout, NULL);

  // initialize randomness
  srand(time(NULL) + rand());  // NOLINT

  std::cout << "Parsing " << argv[1] << "..." << std::endl;

  pfxml::file xml(argv[1]);

  size_t pages = 0;
  size_t stage = 0;

  while (xml.next()) {
    const auto& cur = xml.get();
    if (pages == 50) break;

    if (xml.level() == 2 && strcmp(cur.name, "page") == 0) {
      stage = 1;
    } else if (stage == 1) {
      if (xml.level() == 3 && strcmp(cur.name, "title") == 0) {
        xml.next();
        const auto& textEl = xml.get();
        std::cout << textEl.text << '\t';
      } else if (xml.level() == 3 && strcmp(cur.name, "revision") == 0) {
        stage = 2;
      }
    } else if (stage == 2) {
      if (xml.level() == 4 && strcmp(cur.name, "text") == 0) {
        xml.next();
        const auto& textEl = xml.get();
        printAbstract(textEl.text);
        std::cout << "\n";
        pages++;
      }
    }
  }

  return static_cast<int>(RetCode::SUCCESS);
}
