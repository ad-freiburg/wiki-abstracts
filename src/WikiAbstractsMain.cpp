// Copyright 2019, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Patrick Brosi <brosi@informatik.uni-freiburg.de>

#include <iostream>
#include "pfxml.h"

enum class RetCode { SUCCESS = 0 };

enum TextStage { LBEG,  TEXT, IN_CURL, IN_SQ, IN_BR, IN_IT, IN_H, IN_H_TIT, IN_H_CL, IN_TAG};

std::string getAbstract(const char* text);

// _____________________________________________________________________________
std::string parseSq(const char* str) {
  std::string ret = str;

  std::vector<std::string> split;

  size_t last = 0;
  while (last != std::string::npos) {
    auto pos = ret.find('|', last + 1);
    split.push_back(ret.substr(last, pos - last));
    if (pos != std::string::npos) pos++;
    last = pos;
  }

  if (split.size() == 0) return "";

  size_t pos = split[0].find(':');
  if (pos != std::string::npos) {
    std::string type = split[0].substr(0, pos);
    ret = ret.substr(pos + 1, std::string::npos);

    if (type == "File") return "";
    if (type == "Image") return "";
    if (type == "file") return "";
    if (type == "image") return "";
  }

  return getAbstract(split.back().c_str());
}

// _____________________________________________________________________________
std::string parseCrl(const char* str) {
  // TODO: implement AS OF parser
  return "";
}

// _____________________________________________________________________________
std::string parseXml(const char* tag, const char* content) {
  if (strcmp(tag, "ref") == 0) return "";
  if (strcmp(tag, "math") == 0) return content;
  if (strcmp(tag, "var") == 0) return content;
  return "";
}

// _____________________________________________________________________________
std::string parseBr(const char* str) {
  return "";
}

// _____________________________________________________________________________
std::string getAbstract(const char* text) {
  // TODO: implement AS OF parser
  size_t pos = 0;
  std::string ret;

  TextStage s = LBEG;
  size_t HEAD_D = 0;
  size_t SQ_D = 0;
  size_t CRL_D = 0;
  size_t BR_D = 0;

  std::string tmp;
  std::string tmp2;

  while (text[pos]) {
    switch (s) {
      case LBEG:
        if (text[pos] == '\n') {
          pos++;
          continue;
        } else if (std::isspace(text[pos])) {
          s = TEXT;
          continue;
        } else if (text[pos] == '=') {
          s = IN_H;
          HEAD_D += 1;
          pos++;
          continue;
        } else if (text[pos] == '*' || text[pos] == '#' || text[pos] == ':' || text[pos] == ';'){
          if (strncmp(text + pos, "#REDIRECT", 9) == 0) return "";
          if (strncmp(text + pos, "#redirect", 9) == 0) return "";
          pos++;
          continue;
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
            return ret;
          }
          pos++;
          continue;
        } else {
          s = TEXT;
          continue;
        }

      case IN_CURL:
        if (text[pos] == '}' && text[pos+1] == '}') {
          ret += parseCrl(tmp.c_str());
          pos += 2;
          CRL_D--;
          if (CRL_D == 0) s = TEXT;
          continue;
        } else if (text[pos] == '{' && text[pos+1] == '{') {
          s = IN_CURL;
          tmp += "{{";
          CRL_D++;
          pos += 2;
          continue;
        }
        tmp += text[pos];
        pos++;
        continue;

      case IN_SQ:
        if (text[pos] == ']' && text[pos+1] == ']') {
          ret += parseSq(tmp.c_str());
          pos += 2;
          SQ_D--;
          if (SQ_D == 0) s = TEXT;
          continue;
        } else if (text[pos] == '[' && text[pos+1] == '[') {
          s = IN_SQ;
          tmp += "[[";
          SQ_D++;
          pos += 2;
          continue;
        }
        tmp += text[pos];
        pos++;
        continue;

      case IN_BR:
        if (text[pos] == ')') {
          // delete the space before the bracket
          if (ret.back() == ' ') ret.resize(ret.size() - 1);
          ret += parseBr(tmp.c_str());
          pos++;
          BR_D--;
          if (BR_D == 0) s = TEXT;
          continue;
        } else if (text[pos] == '(') {
          s = IN_BR;
          tmp += "(";
          BR_D++;
          pos++;
          continue;
        }
        tmp += text[pos];
        pos++;
        continue;

      case IN_TAG:
        if (text[pos] == '<' && text[pos+1] == '/') {
          std::string locTmp;
          size_t p = pos + 1;
          while (text[p]) {
            p++;
            if (text[p] == '\n') {
              s = TEXT;
              tmp.clear();
              tmp2.clear();
              break;
            } else if (text[p] == '>') {
              pos = p;
              if (locTmp == tmp) {
                ret += parseXml(tmp.c_str(), tmp2.c_str());
                tmp.clear();
                tmp2.clear();
                pos = p + 1;
                s = TEXT;
                break;
              }
            } else {
              locTmp += text[p];
            }
          }
          continue;
        } else {
          tmp2 += text[pos];
          pos++;
          continue;
        }

      case TEXT:
        if (text[pos] == '\n') {
          // avoid double spaces
          if (ret.back() != ' ') ret += ' ';
          pos++;
          s = LBEG;
          continue;
        } else if (text[pos] == '\'') {
          pos++;
          continue;
        } else if (text[pos] == '<' && text[pos+1] == '!' && text[pos+2] == '-' && text[pos+3] == '-') {
          // comment
          size_t p = pos + 3;
          while (true) {
            p++;
            if (!text[p]) {
              pos = p;
              break;
            } else if (text[p] == '-' && text[p+1] == '-' && text[p+2] == '>') {
              pos = p + 3;
              break;
            }
          }
          s = TEXT;
          continue;
        } else if (text[pos] == '<') {
          s = IN_TAG;
          tmp.clear();
          tmp2.clear();
          size_t p = pos;
          while (text[p]) {
            p++;
            if (text[p] == '\n') {
              s = TEXT;
              tmp.clear();
              tmp2.clear();
              break;
            } else if (text[p] == '>') {
              pos = p;
              tmp = tmp.substr(0, tmp.find(' '));
              break;
            } else if (text[p] == '/' && text[p+1] == '>') {
              pos = p + 1;
              tmp.clear();
              tmp2.clear();
              s = TEXT;
              break;
            } else {
              tmp += text[p];
            }
          }

          pos++;
          continue;
        } else {
          if (text[pos] == '{' && text[pos+1] == '{') {
            s = IN_CURL;
            CRL_D = 1;
            tmp.clear();
            pos += 2;
            continue;
          } else if (text[pos] == '[' && text[pos+1] == '[') {
            s = IN_SQ;
            SQ_D = 1;
            tmp.clear();
            pos += 2;
            continue;
          } else if (text[pos] == '(') {
            s = IN_BR;
            BR_D = 1;
            tmp.clear();
            pos += 1;
            continue;
          }
        }

        if (text[pos] != ' ' || ret.back() != ' ') ret += text[pos];
        pos++;
        continue;
    }
  }

  return ret;
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

  std::string title;

  while (xml.next()) {
    const auto& cur = xml.get();
    // if (pages == 5) break;

    if (xml.level() == 2 && strcmp(cur.name, "page") == 0) {
      stage = 1;
    } else if (stage == 1) {
      if (xml.level() == 3 && strcmp(cur.name, "title") == 0) {
        xml.next();
        const auto& textEl = xml.get();
        title = textEl.text;
      } else if (xml.level() == 3 && strcmp(cur.name, "revision") == 0) {
        stage = 2;
      }
    } else if (stage == 2) {
      if (xml.level() == 4 && strcmp(cur.name, "text") == 0) {
        xml.next();
        const auto& textEl = xml.get();
        auto abstr = pfxml::file::decode(getAbstract(textEl.text).c_str());
        abstr = getAbstract(abstr.c_str());
        if (abstr.size()) {
          std::cout << title << '\t' << abstr << "\n";
          pages++;
        }
      }
    }
  }

  return static_cast<int>(RetCode::SUCCESS);
}
