// Copyright 2019, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Patrick Brosi <brosi@informatik.uni-freiburg.de>

#include <iostream>
#include <set>
#include "pfxml.h"

enum class RetCode { SUCCESS = 0, MISSING_WIKI_DUMP = 1, PARSE_ERROR = 2 };

enum TextStage {
  LBEG,
  TEXT,
  IN_CURL,
  IN_TABLE,
  IN_SQ,
  IN_SSQ,
  IN_BR,
  IN_H,
  IN_H_TIT,
  IN_H_CL,
  IN_TAG
};

// namespaces which should be dropped
static const std::set<std::string> DROP_NS = {
    "User",
    "Wikipedia",
    "File",
    "MediaWiki",
    "Template",
    "Help",
    "Category",
    "Portal",
    "Book",
    "Draft",
    "TimedText",
    "Module",
    "Education Program",
    "Gadget",
    "Gadget definition",
    "Special",
    "Media"
};

std::string parse(const char* text, size_t maxParas, bool woBr);

// _____________________________________________________________________________
bool usePage(const std::string& tit) {
  // returns true if a page should be used, based on title

  auto nsPos = tit.find(':');
  if (nsPos != std::string::npos) {
    auto ns = tit.substr(0, nsPos);
    if (DROP_NS.count(ns)) return false;
  }
  return true;
}

// _____________________________________________________________________________
std::string parseSSq(const char* str) {
  // parse a single squared command like [http://google.de], usually used
  // for external links

  std::string ret = str;

  std::vector<std::string> split;

  size_t last = 0;
  while (last != std::string::npos) {
    auto pos = ret.find(' ', last + 1);
    split.push_back(ret.substr(last, pos - last));
    if (pos != std::string::npos) pos++;
    last = pos;
  }

  if (split.size() == 0) return "";

  return parse(split.back().c_str(), 1, true);
}

// _____________________________________________________________________________
std::string parseSq(const char* str) {
  // parse a double squared command like [[Freiburg im Breisgau]], usually used
  // for internal links

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

    if (type == "File") return "";
    if (type == "Image") return "";
    if (type == "file") return "";
    if (type == "image") return "";
  }

  if (split.size() == 1) {
    size_t pos = split[0].find(':');
    if (pos != std::string::npos) {
      ret = ret.substr(pos + 1, std::string::npos);
    }

    // wikipedia auto-hides stuff after comma
    pos = ret.find(',');
    if (pos != std::string::npos) {
      ret = ret.substr(0, pos);
    }

    return parse(ret.c_str(), 1, true);
  }

  return parse(split.back().c_str(), 1, true);
}

// _____________________________________________________________________________
std::pair<std::string, size_t> parseCrl(const char* str) {
  // parse a template

  if (strcmp(str, "disambiguation") == 0) return {"", 1};
  if (strcmp(str, "DISAMBIGUATION") == 0) return {"", 1};
  if (strcmp(str, "Disambiguation") == 0) return {"", 1};

  if (strcmp(str, "human name disambiguation") == 0) return {"", 1};
  if (strcmp(str, "HUMAN NAME DISAMBIGUATION") == 0) return {"", 1};
  if (strcmp(str, "Human Name Disambiguation") == 0) return {"", 1};

  std::vector<std::string> split;

  size_t last = 0;
  std::string ret = str;
  while (last != std::string::npos) {
    auto pos = ret.find('|', last + 1);
    split.push_back(ret.substr(last, pos - last));
    if (pos != std::string::npos) pos++;
    last = pos;
  }

  if (split.size() > 1) {
    if (split[0] == "math") return {parse(split[1].c_str(), 1, false), 0};
  }

  return {"", 0};
}

// _____________________________________________________________________________
std::string parseXml(const char* tag, const char* content) {
  // parse xml found in the wikitext

  if (strcmp(tag, "ref") == 0) return "";
  if (strcmp(tag, "math") == 0) return content;
  if (strcmp(tag, "var") == 0) return content;
  return "";
}

// _____________________________________________________________________________
std::string parseBr(const char* str, bool woBr) {
  if (woBr) return "";

  std::stringstream ret;
  // with a leading space!
  ret << " (" << parse(str, 1, false) << ")";
  return ret.str();
}

// _____________________________________________________________________________
std::string parse(const char* text, size_t maxParas, bool woBr) {
  size_t pos = 0;
  std::string ret;

  TextStage s = LBEG;
  size_t HEAD_D = 0;
  size_t HEAD_D_ORIG = 0;
  size_t SQ_D = 0;
  size_t SSQ_D = 0;
  size_t CRL_D = 0;
  size_t TBL_D = 0;
  size_t BR_D = 0;

  std::string tmp;
  std::string tmp2;

  size_t paras = 0;

  while (text[pos]) {
    switch (s) {
      case LBEG:
        if (text[pos] == '\n') {
          if (ret.size()) paras++;
          if (paras >= maxParas) return ret;
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
        } else if (text[pos] == '*' || text[pos] == '#' || text[pos] == ':' ||
                   text[pos] == ';') {
          if (strncmp(text + pos, "#REDIRECT", 9) == 0) return "";
          if (strncmp(text + pos, "#redirect", 9) == 0) return "";
          if (strncmp(text + pos, "#Redirect", 9) == 0) return "";
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
          HEAD_D_ORIG = HEAD_D;
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
          // = wasn't the header closing, but part of the header
          HEAD_D = HEAD_D_ORIG;
          s = IN_H_TIT;
          pos++;
          continue;
        }

      case IN_TABLE:
        if (text[pos] == '|' && text[pos + 1] == '}') {
          pos += 2;
          TBL_D--;
          if (TBL_D == 0) s = TEXT;
          continue;
        } else if (text[pos] == '{' && text[pos + 1] == '|') {
          s = IN_TABLE;
          TBL_D++;
          pos += 2;
          continue;
        }
        pos++;
        continue;

      case IN_CURL:
        if (text[pos] == '}' && text[pos + 1] == '}') {
          auto crl = parseCrl(tmp.c_str());

          // signal: abort!
          if (crl.second == 1) return "";
          ret += crl.first;
          pos += 2;
          CRL_D--;
          if (CRL_D == 0) s = TEXT;
          continue;
        } else if (text[pos] == '{' && text[pos + 1] == '{') {
          s = IN_CURL;
          tmp += "{{";
          CRL_D++;
          pos += 2;
          continue;
        }
        tmp += text[pos];
        pos++;
        continue;

      case IN_SSQ:
        if (text[pos] == ']') {
          ret += parseSSq(tmp.c_str());
          pos += 1;
          SSQ_D--;
          if (SSQ_D == 0) s = TEXT;
          continue;
        } else if (text[pos] == '[') {
          s = IN_SSQ;
          tmp += "[";
          SSQ_D++;
          pos += 1;
          continue;
        }
        tmp += text[pos];
        pos++;
        continue;

      case IN_SQ:
        if (text[pos] == ']' && text[pos + 1] == ']') {
          ret += parseSq(tmp.c_str());
          pos += 2;
          SQ_D--;
          if (SQ_D == 0) s = TEXT;
          continue;
        } else if (text[pos] == '[' && text[pos + 1] == '[') {
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
          ret += parseBr(tmp.c_str(), woBr);
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
        if (text[pos] == '<' && text[pos + 1] == '/') {
          std::string locTmp;
          size_t p = pos + 1;
          while (text[p]) {
            p++;
            if (text[p] == '\n') {
              s = TEXT;
              tmp.clear();
              tmp2.clear();
              break;
            } else if (text[p] == '>' || text[p] == 0) {
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
        } else if (strncmp(text + pos, "__TOC__", 7) == 0) {
          return ret;
        } else if (strncmp(text + pos, "__FORCETOC__", 12) == 0) {
          return ret;
        } else if (strncmp(text + pos, "__NOTOC__", 9) == 0) {
          pos += 9;
          continue;
        } else if (text[pos] == '\'') {
          pos++;
          continue;
        } else if (text[pos] == '<' && text[pos + 1] == '!' &&
                   text[pos + 2] == '-' && text[pos + 3] == '-') {
          // comment
          size_t p = pos + 3;
          while (true) {
            p++;
            if (!text[p]) {
              pos = p;
              break;
            } else if (text[p] == '-' && text[p + 1] == '-' &&
                       text[p + 2] == '>') {
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
            } else if (text[p] == '/' && text[p + 1] == '>') {
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
          if (text[pos] == '{' && text[pos + 1] == '{') {
            s = IN_CURL;
            CRL_D = 1;
            tmp.clear();
            pos += 2;
            continue;
          } else if (text[pos] == '{' && text[pos + 1] == '|') {
            s = IN_TABLE;
            TBL_D = 1;
            tmp.clear();
            pos += 2;
            continue;
          } else if (text[pos] == '[' && text[pos + 1] == '[') {
            s = IN_SQ;
            SQ_D = 1;
            tmp.clear();
            pos += 2;
            continue;
          } else if (text[pos] == '[') {
            s = IN_SSQ;
            SSQ_D = 1;
            tmp.clear();
            pos += 1;
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

  if (paras > 1) return parse(text, 1, woBr);
  return ret;
}

// _____________________________________________________________________________
int main(int argc, char** argv) {
  // disable output buffering for standard output
  setbuf(stdout, NULL);

  // initialize randomness
  srand(time(NULL) + rand());  // NOLINT

  if (argc < 2) {
    std::cerr << "No Wikipedia dump XML file given.\n\n";
  }

  if (argc < 2 || !strcmp(argv[1], "--help")) {
    std::cout << "Usage: \n  " << argv[0] << " <wikipedia dump>" << std::endl;
    return static_cast<int>(argc < 2 ? RetCode::MISSING_WIKI_DUMP
                                     : RetCode::SUCCESS);
  }

  try {
  pfxml::file xml(argv[1]);

  size_t stage = 0;

  std::string title;

  while (xml.next()) {
    const auto& cur = xml.get();
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
        auto abstr = pfxml::file::decode(parse(textEl.text, 10, true).c_str());

        // decode two times, because the decoded text may be XML again...
        abstr = pfxml::file::decode(abstr);
        abstr = parse(abstr.c_str(), 10, false);

        // output if page is used
        if (abstr.size() && usePage(title)) {
          std::cout << pfxml::file::decode(title) << '\t' << abstr << "\n";
        }
      }
    }
  }
  } catch (const pfxml::parse_exc& e) {
    std::cerr << e.what() << std::endl;
    return static_cast <int>(RetCode::PARSE_ERROR);
  }

  return static_cast<int>(RetCode::SUCCESS);
}
