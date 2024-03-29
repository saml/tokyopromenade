/*************************************************************************************************
 * The utility API of Tokyo Promenade
 *                                                      Copyright (C) 2008-2012 Mikio Hirabayashi
 * This file is part of Tokyo Promenade.
 * This program is free software: you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation, either version
 * 3 of the License, or any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************************************/


#include "common.h"

#define HEADLVMAX      6                 // maximum level of header
#define SPACELVMAX     8                 // maximum level of spacer
#define IMAGELVMAX     6                 // maximum level of image



/*************************************************************************************************
 * API
 *************************************************************************************************/


/* Load a Wiki string. */
void wikiload(TCMAP *cols, const char *str){
  assert(cols && str);
  TCLIST *lines = tcstrsplit(tcstrskipspc(str), "\n");
  int lnum = tclistnum(lines);
  int size = 0;
  int bottom = 0;
  for(int i = 0; i < lnum; i++){
    int lsiz;
    const char *line = tclistval(lines, i, &lsiz);
    size += lsiz;
    if(*line != '#' && *tcstrskipspc(line) != '\0') bottom = i;
  }
  TCXSTR *text = tcxstrnew3(size + 1);
  TCXSTR *comments = tcxstrnew3(size + 1);
  int64_t xdate = INT64_MIN;
  bool body = false;
  char numbuf[NUMBUFSIZ];
  for(int i = 0; i < lnum; i++){
    int lsiz;
    const char *line = tclistval(lines, i, &lsiz);
    if(lsiz > 0 && line[lsiz-1] == '\r'){
      lsiz--;
      ((char *)line)[lsiz] = '\0';
    }
    if(*line == '#'){
      const char *rp = line + 1;
      if(*rp == ':'){
        int64_t id = atoi(rp + 1);
        if(id > 0){
          sprintf(numbuf, "%lld", (long long)id);
          tcmapputkeep2(cols, "id", numbuf);
        }
      } else if(*rp == '!' && !body){
        rp = tcstrsqzspc((char *)rp + 1);
        if(*rp != '\0') tcmapputkeep2(cols, "name", rp);
      } else if(*rp == 'c' && !body){
        int64_t date = tcstrmktime(rp + 1);
        if(date != INT64_MIN){
          sprintf(numbuf, "%lld", (long long)date);
          tcmapputkeep2(cols, "cdate", numbuf);
          if(date > xdate) xdate = date;
        }
      } else if(*rp == 'm' && !body){
        int64_t date = tcstrmktime(rp + 1);
        if(date != INT64_MIN){
          sprintf(numbuf, "%lld", (long long)date);
          tcmapputkeep2(cols, "mdate", numbuf);
          if(date > xdate) xdate = date;
        }
      } else if(*rp == 'o' && !body){
        rp = tcstrsqzspc((char *)rp + 1);
        if(*rp != '\0') tcmapputkeep2(cols, "owner", rp);
      } else if(*rp == 't' && !body){
        rp = tcstrsqzspc((char *)rp + 1);
        if(*rp != '\0') tcmapputkeep2(cols, "tags", rp);
      } else if(*rp == '%' && i > bottom){
        rp++;
        char *co = strchr(rp, '|');
        if(co){
          *(co++) = '\0';
          char *ct = strchr(co, '|');
          if(ct){
            *(ct++) = '\0';
            int64_t date = tcstrmktime(rp);
            tcstrtrim(co);
            tcstrtrim(ct);
            if(date != INT64_MIN && *co != '\0' && *ct != '\0'){
              tcxstrprintf(comments, "%lld|%s|%s\n", (long long)date, co, ct);
              if(date > xdate) xdate = date;
            }
          }
        }
      } else if(*rp == '-' || body){
        tcxstrcat(text, line, lsiz);
        tcxstrcat(text, "\n", 1);
      }
    } else if(lsiz > 0 || body){
      tcxstrcat(text, line, lsiz);
      tcxstrcat(text, "\n", 1);
      body = true;
    }
  }
  const char *tbuf = tcxstrptr(text);
  int tsiz = tcxstrsize(text);
  while(tsiz > 0 && tbuf[tsiz-1] == '\n'){
    tsiz--;
  }
  if(tbuf[tsiz] == '\n') tsiz++;
  tcmapputkeep(cols, "text", 4, tbuf, tsiz);
  if(tcxstrsize(comments) > 0)
    tcmapputkeep(cols, "comments", 8, tcxstrptr(comments), tcxstrsize(comments));
  if(xdate != INT64_MIN) tcmapprintf(cols, "xdate", "%lld", (long long)xdate);
  tcxstrdel(comments);
  tcxstrdel(text);
  tclistdel(lines);
}


/* Dump the attributes and the body text of an article into a Wiki string. */
void wikidump(TCXSTR *rbuf, TCMAP *cols){
  assert(rbuf && cols);
  if(tcmaprnum(cols) > 0){
    char numbuf[NUMBUFSIZ];
    const char *val = tcmapget2(cols, "id");
    if(val){
      tcxstrcat(rbuf, "#: ", 3);
      tcxstrcat2(rbuf, val);
      tcxstrcat(rbuf, "\n", 1);
    }
    val = tcmapget2(cols, "name");
    if(val){
      tcxstrcat(rbuf, "#! ", 3);
      tcxstrcat2(rbuf, val);
      tcxstrcat(rbuf, "\n", 1);
    }
    val = tcmapget2(cols, "cdate");
    if(val){
      tcdatestrwww(tcatoi(val), INT_MAX, numbuf);
      tcxstrcat(rbuf, "#c ", 3);
      tcxstrcat2(rbuf, numbuf);
      tcxstrcat(rbuf, "\n", 1);
    }
    val = tcmapget2(cols, "mdate");
    if(val){
      tcdatestrwww(tcatoi(val), INT_MAX, numbuf);
      tcxstrcat(rbuf, "#m ", 3);
      tcxstrcat2(rbuf, numbuf);
      tcxstrcat(rbuf, "\n", 1);
    }
    val = tcmapget2(cols, "owner");
    if(val){
      tcxstrcat(rbuf, "#o ", 3);
      tcxstrcat2(rbuf, val);
      tcxstrcat(rbuf, "\n", 1);
    }
    val = tcmapget2(cols, "tags");
    if(val){
      tcxstrcat(rbuf, "#t ", 3);
      tcxstrcat2(rbuf, val);
      tcxstrcat(rbuf, "\n", 1);
    }
    tcxstrcat(rbuf, "\n", 1);
  }
  int tsiz;
  const char *tbuf = tcmapget(cols, "text", 4, &tsiz);
  if(tbuf && tsiz > 0) tcxstrcat(rbuf, tbuf, tsiz);
  int csiz;
  const char *cbuf = tcmapget(cols, "comments", 8, &csiz);
  if(cbuf && csiz > 0){
    TCLIST *lines = tcstrsplit(cbuf, "\n");
    int cnum = tclistnum(lines);
    if(cnum > 0){
      tcxstrcat(rbuf, "\n", 1);
      for(int i = 0; i < cnum; i++){
        const char *rp = tclistval2(lines, i);
        char *co = strchr(rp, '|');
        if(co){
          *(co++) = '\0';
          char *ct = strchr(co, '|');
          if(ct){
            *(ct++) = '\0';
            char numbuf[NUMBUFSIZ];
            tcdatestrwww(tcatoi(rp), INT_MAX, numbuf);
            tcxstrcat(rbuf, "#% ", 3);
            tcxstrcat2(rbuf, numbuf);
            tcxstrcat(rbuf, "|", 1);
            tcxstrcat2(rbuf, co);
            tcxstrcat(rbuf, "|", 1);
            tcxstrcat2(rbuf, ct);
            tcxstrcat(rbuf, "\n", 1);
          }
        }
      }
    }
    tclistdel(lines);
  }
}


/* Dump the attributes and the body text of an article into a plain text string. */
void wikidumptext(TCXSTR *rbuf, TCMAP *cols){
  assert(rbuf && cols);
  if(tcmaprnum(cols) > 0){
    char numbuf[NUMBUFSIZ];
    const char *val = tcmapget2(cols, "id");
    if(val) tcxstrprintf(rbuf, "ID: %lld\n", (long long)tcatoi(val));
    val = tcmapget2(cols, "name");
    if(val) tcxstrprintf(rbuf, "Name: %s\n", val);
    val = tcmapget2(cols, "cdate");
    if(val){
      tcdatestrwww(tcatoi(val), INT_MAX, numbuf);
      tcxstrprintf(rbuf, "Creation Date: %s\n", numbuf);
    }
    val = tcmapget2(cols, "mdate");
    if(val){
      tcdatestrwww(tcatoi(val), INT_MAX, numbuf);
      tcxstrprintf(rbuf, "Modification Date: %s\n", numbuf);
    }
    val = tcmapget2(cols, "owner");
    if(val) tcxstrprintf(rbuf, "Owner: %s\n", val);
    val = tcmapget2(cols, "tags");
    if(val) tcxstrprintf(rbuf, "Tags: %s\n", val);
    tcxstrcat2(rbuf, "----\n\n");
  }
  const char *text = tcmapget2(cols, "text");
  if(text) wikitotext(rbuf, text);
  const char *com = tcmapget2(cols, "comments");
  if(com){
    TCLIST *lines = tcstrsplit(com, "\n");
    int cnum = tclistnum(lines);
    if(cnum > 0){
      for(int i = 0; i < cnum; i++){
        const char *rp = tclistval2(lines, i);
        char *co = strchr(rp, '|');
        if(co){
          *(co++) = '\0';
          char *ct = strchr(co, '|');
          if(ct){
            *(ct++) = '\0';
            char numbuf[NUMBUFSIZ];
            tcdatestrwww(tcatoi(rp), INT_MAX, numbuf);
            tcxstrprintf(rbuf, "# ");
            tcxstrprintf(rbuf, "[%s]: %s: ", numbuf, co);
            wikitotextinline(rbuf, ct);
            tcxstrcat(rbuf, "\n", 1);
          }
        }
      }
    }
    tclistdel(lines);
  }
}


/* Convert a Wiki string into a plain text string. */
void wikitotext(TCXSTR *rbuf, const char *str){
  assert(rbuf && str);
  TCLIST *lines = tcstrsplit(str, "\n");
  int lnum = tclistnum(lines);
  int ri = 0;
  while(ri < lnum){
    int lsiz;
    const char *line = tclistval(lines, ri, &lsiz);
    if(lsiz > 0 && line[lsiz-1] == '\r'){
      lsiz--;
      ((char *)line)[lsiz] = '\0';
    }
    if(*line == '#'){
      ri++;
    } else if(*line == '*'){
      int lv = 1;
      const char *rp = line + 1;
      while(*rp == '*'){
        lv++;
        rp++;
      }
      rp = tcstrskipspc(rp);
      if(*rp != '\0'){
        if(lv == 1){
          tcxstrcat2(rbuf, "[[[ ");
          wikitotextinline(rbuf, rp);
          tcxstrcat2(rbuf, " ]]]\n\n");
        } else if(lv == 2){
          tcxstrcat2(rbuf, "[[ ");
          wikitotextinline(rbuf, rp);
          tcxstrcat2(rbuf, " ]]\n\n");
        } else if(lv == 3){
          tcxstrcat2(rbuf, "[ ");
          wikitotextinline(rbuf, rp);
          tcxstrcat2(rbuf, " ]\n\n");
        } else {
          wikitotextinline(rbuf, rp);
          tcxstrcat2(rbuf, "\n\n");
        }
      }
      ri++;
    } else if(*line == '-' || *line == '+'){
      int ei = ri + 1;
      while(ei < lnum){
        const char *rp = tclistval2(lines, ei);
        if(*rp != '-' && *rp != '+') break;
        ei++;
      }
      for(int i = ri; i < ei; i++){
        const char *rp = tclistval2(lines, i);
        int sep = *rp;
        int clv = 1;
        rp++;
        while(*rp == sep){
          clv++;
          rp++;
        }
        rp = tcstrskipspc(rp);
        for(int j = 0; j < clv; j++){
          tcxstrcat2(rbuf, "  ");
        }
        tcxstrcat2(rbuf, "* ");
        wikitotextinline(rbuf, rp);
        tcxstrcat2(rbuf, "\n");
      }
      tcxstrcat2(rbuf, "\n");
      ri = ei;
    } else if(*line == ',' || *line == '|'){
      int ei = ri + 1;
      while(ei < lnum){
        const char *rp = tclistval2(lines, ei);
        if(*rp != ',' && *rp != '|') break;
        ei++;
      }
      for(int i = ri; i < ei; i++){
        const char *rp = tclistval2(lines, i);
        int sep = *rp;
        rp++;
        tcxstrcat2(rbuf, "  ");
        for(int j = 0; true; j++){
          if(j > 0) tcxstrcat2(rbuf, " | ");
          const char *pv = strchr(rp, sep);
          char *field = pv ? tcmemdup(rp, pv - rp) : tcstrdup(rp);
          tcstrtrim(field);
          wikitotextinline(rbuf, field);
          tcfree(field);
          if(!pv) break;
          rp = pv + 1;
        }
        tcxstrcat2(rbuf, "\n");
      }
      tcxstrcat2(rbuf, "\n");
      ri = ei;
    } else if(*line == '>'){
      int ei = ri + 1;
      while(ei < lnum){
        const char *rp = tclistval2(lines, ei);
        if(*rp != '>') break;
        ei++;
      }
      for(int i = ri; i < ei; i++){
        const char *rp = tclistval2(lines, i);
        rp = tcstrskipspc(rp + 1);
        if(*rp != '\0'){
          tcxstrcat2(rbuf, ">> ");
          wikitotextinline(rbuf, rp);
          tcxstrcat2(rbuf, "\n\n");
        }
      }
      ri = ei;
    } else if(tcstrfwm(line, "{{{")){
      TCXSTR *sep = tcxstrnew();
      line += 3;
      while(*line != '\0'){
        switch(*line){
          case '{': tcxstrprintf(sep, "%c", '}'); break;
          case '[': tcxstrprintf(sep, "%c", ']'); break;
          case '<': tcxstrprintf(sep, "%c", '>'); break;
          case '(': tcxstrprintf(sep, "%c", ')'); break;
          default: tcxstrcat(sep, line, 1); break;
        }
        line++;
      }
      tcxstrcat(sep, "}}}", 3);
      const char *sepstr = tcxstrptr(sep);
      ri++;
      int ei = ri;
      while(ei < lnum){
        const char *rp = tclistval2(lines, ei);
        if(!strcmp(rp, sepstr)) break;
        ei++;
      }
      for(int i = ri; i < ei; i++){
        const char *rp = tclistval2(lines, i);
        tcxstrprintf(rbuf, "  %s\n", rp);
      }
      tcxstrcat(rbuf, "\n", 1);
      tcxstrdel(sep);
      ri = ei + 1;
    } else if(*line == '@'){
      line++;
      if(*line == '@') line++;
      bool obj = false;
      if(*line == '!'){
        obj = true;
        line++;
      }
      while(*line == '<' || *line == '>' || *line == '+' || *line == '-' || *line == '|'){
        line++;
      }
      line = tcstrskipspc(line);
      if(*line != '\0') tcxstrprintf(rbuf, "  [%s:%s]\n\n", obj ? "OBJECT" : "IMAGE", line);
      ri++;
    } else if(tcstrfwm(line, "===")){
      tcxstrcat2(rbuf, "----\n\n");
      ri++;
    } else {
      line = tcstrskipspc(line);
      if(*line != '\0'){
        tcxstrcat2(rbuf, "  ");
        wikitotextinline(rbuf, line);
        tcxstrcat2(rbuf, "\n\n");
        ri++;
      } else {
        ri++;
      }
    }
  }
  tclistdel(lines);
}


/* Add an inline Wiki string into plain text.
   `rbuf' specifies the result buffer.
   `line' specifies the inline Wiki string. */
void wikitotextinline(TCXSTR *rbuf, const char *line){
  assert(rbuf && line);
  while(*line != '\0'){
    const char *pv;
    if(*line == '['){
      if(tcstrfwm(line, "[[") && (pv = strstr(line + 2, "]]")) != NULL){
        char *field = tcmemdup(line + 2, pv - line - 2);
        const char *uri = field;
        char *sep = strchr(field, '|');
        if(sep){
          *sep = '\0';
          uri = sep + 1;
        }
        wikitotextinline(rbuf, field);
        if(strcmp(field, uri)) tcxstrprintf(rbuf, "(%s)", uri);
        tcfree(field);
        line = pv + 2;
      } else if(tcstrfwm(line, "[*") && (pv = strstr(line + 2, "*]")) != NULL){
        char *field = tcmemdup(line + 2, pv - line - 2);
        tcxstrcat2(rbuf, "__");
        wikitotextinline(rbuf, field);
        tcxstrcat2(rbuf, "__");
        tcfree(field);
        line = pv + 2;
      } else if(tcstrfwm(line, "[\"") && (pv = strstr(line + 2, "\"]")) != NULL){
        char *field = tcmemdup(line + 2, pv - line - 2);
        tcxstrcat2(rbuf, " \"");
        wikitotextinline(rbuf, field);
        tcxstrcat2(rbuf, "\" ");
        tcfree(field);
        line = pv + 2;
      } else if(tcstrfwm(line, "[+") && (pv = strstr(line + 2, "+]")) != NULL){
        char *field = tcmemdup(line + 2, pv - line - 2);
        tcxstrcat2(rbuf, "++");
        wikitotextinline(rbuf, field);
        tcxstrcat2(rbuf, "++");
        tcfree(field);
        line = pv + 2;
      } else if(tcstrfwm(line, "[-") && (pv = strstr(line + 2, "-]")) != NULL){
        char *field = tcmemdup(line + 2, pv - line - 2);
        tcxstrcat2(rbuf, "--");
        wikitotextinline(rbuf, field);
        tcxstrcat2(rbuf, "--");
        tcfree(field);
        line = pv + 2;
      } else if(tcstrfwm(line, "[#") && (pv = strstr(line + 2, "#]")) != NULL){
        char *field = tcmemdup(line + 2, pv - line - 2);
        tcxstrcat2(rbuf, "##");
        wikitotextinline(rbuf, field);
        tcxstrcat2(rbuf, "##");
        tcfree(field);
        line = pv + 2;
      } else if(tcstrfwm(line, "[$") && (pv = strstr(line + 2, "$]")) != NULL){
        char *field = tcmemdup(line + 2, pv - line - 2);
        tcxstrcat2(rbuf, "$$");
        wikitotextinline(rbuf, field);
        tcxstrcat2(rbuf, "$$");
        tcfree(field);
        line = pv + 2;
      } else if(tcstrfwm(line, "[=") && (pv = strstr(line + 2, "=]")) != NULL){
        char *field = tcmemdup(line + 2, pv - line - 2);
        tcxstrprintf(rbuf, "%s", field);
        tcfree(field);
        line = pv + 2;
      } else {
        tcxstrcat(rbuf, line, 1);
        line++;
      }
    } else {
      tcxstrcat(rbuf, line, 1);
      line++;
    }
  }
}


/* Dump the attributes and the body text of an article into an HTML string. */
void wikidumphtml(TCXSTR *rbuf, TCMAP *cols, const char *buri, int bhl, const char *duri){
  assert(rbuf && cols && buri && bhl >= 0);
  char idbuf[NUMBUFSIZ];
  const char *val = tcmapget2(cols, "id");
  if(val){
    sprintf(idbuf, "article%lld", (long long)tcatoi(val));
    tcxstrprintf(rbuf, "<div class=\"article\" id=\"%@\">\n", idbuf);
  } else {
    *idbuf = '\0';
    tcxstrprintf(rbuf, "<div class=\"article\">\n");
  }
  if(tcmaprnum(cols) > 0){
    tcxstrprintf(rbuf, "<div class=\"attributes\">\n");
    char numbuf[NUMBUFSIZ];
    val = tcmapget2(cols, "name");
    if(val) tcxstrprintf(rbuf, "<h%d class=\"attr ah0\">%@</h%d>\n", bhl + 1, val, bhl + 1);
    const char *val = tcmapget2(cols, "id");
    if(val) tcxstrprintf(rbuf, "<div class=\"attr\">ID:"
                         " <span class=\"id\">%lld</span></div>\n", (long long)tcatoi(val));
    val = tcmapget2(cols, "cdate");
    if(val){
      tcdatestrwww(tcatoi(val), INT_MAX, numbuf);
      tcxstrprintf(rbuf, "<div class=\"attr\">Creation Date:"
                   " <span class=\"cdate\">%@</span></div>\n", numbuf);
    }
    val = tcmapget2(cols, "mdate");
    if(val){
      tcdatestrwww(tcatoi(val), INT_MAX, numbuf);
      tcxstrprintf(rbuf, "<div class=\"attr\">Modification Date:"
                   " <span class=\"mdate\">%@</span></div>\n", numbuf);
    }
    val = tcmapget2(cols, "owner");
    if(val) tcxstrprintf(rbuf, "<div class=\"attr\">Owner:"
                         " <span class=\"owner\">%@</span></div>\n", val);
    val = tcmapget2(cols, "tags");
    if(val) tcxstrprintf(rbuf, "<div class=\"attr\">Tags:"
                         " <span class=\"tags\">%@</span></div>\n", val);
    tcxstrcat2(rbuf, "</div>\n");
  }
  const char *text = tcmapget2(cols, "text");
  if(text && *text != '\0'){
    tcxstrprintf(rbuf, "<div class=\"text\">\n");
    wikitohtml(rbuf, text, *idbuf != '\0' ? idbuf : NULL, buri, bhl + 1, duri, -1, -1);
    tcxstrcat2(rbuf, "</div>\n");
  }
  const char *com = tcmapget2(cols, "comments");
  if(com && *com != '\0'){
    TCLIST *lines = tcstrsplit(com, "\n");
    int cnum = tclistnum(lines);
    if(cnum > 0){
      tcxstrprintf(rbuf, "<div class=\"comments\">\n");
      int cnt = 0;
      for(int i = 0; i < cnum; i++){
        const char *rp = tclistval2(lines, i);
        char *co = strchr(rp, '|');
        if(co){
          *(co++) = '\0';
          char *ct = strchr(co, '|');
          if(ct){
            *(ct++) = '\0';
            char numbuf[NUMBUFSIZ];
            tcdatestrwww(tcatoi(rp), INT_MAX, numbuf);
            cnt++;
            tcxstrprintf(rbuf, "<div class=\"comment\"");
            if(*idbuf != '\0') tcxstrprintf(rbuf, " id=\"%@_c%d\"", idbuf, cnt);
            tcxstrprintf(rbuf, ">\n");
            tcxstrprintf(rbuf, "<span class=\"date\">%@</span> :\n", numbuf);
            tcxstrprintf(rbuf, "<span class=\"owner\">%@</span> :\n", co);
            tcxstrprintf(rbuf, "<span class=\"text\">");
            wikitohtmlinline(rbuf, ct, buri, duri);
            tcxstrprintf(rbuf, "</span>\n");
            tcxstrprintf(rbuf, "</div>\n");
          }
        }
      }
      tcxstrcat2(rbuf, "</div>\n");
    }
    tclistdel(lines);
  }
  tcxstrcat2(rbuf, "</div>\n");
}


/* Convert a Wiki string into an HTML string. */
int wikitohtml(TCXSTR *rbuf, const char *str, const char *id, const char *buri, int bhl,
               const char *duri, int abslen, int absobjnum) {
  assert(rbuf && str && buri && bhl >= 0);
  if(abslen < 0) abslen = INT_MAX;
  if(absobjnum < 0) absobjnum = INT_MAX;
  TCLIST *lines = tcstrsplit(str, "\n");
  int lnum = tclistnum(lines);
  int headcnts[HEADLVMAX];
  memset(headcnts, 0, sizeof(headcnts));
  int txtlen = 0;
  int objnum = 0;
  int tblcnt = 0;
  int imgcnt = 0;
  int ri = 0;
  while(ri < lnum && txtlen < abslen){
    int lsiz;
    const char *line = tclistval(lines, ri, &lsiz);
    if(lsiz > 0 && line[lsiz-1] == '\r'){
      lsiz--;
      ((char *)line)[lsiz] = '\0';
    }
    if(*line == '#'){
      ri++;
    } else if(*line == '*'){
      int lv = 1;
      const char *rp = line + 1;
      while(*rp == '*'){
        lv++;
        rp++;
      }
      int abslv = lv;
      lv += bhl;
      if(lv > HEADLVMAX) lv = HEADLVMAX;
      rp = tcstrskipspc(rp);
      if(*rp != '\0'){
        headcnts[lv-1]++;
        tcxstrprintf(rbuf, "<h%d", lv);
        if(id){
          tcxstrprintf(rbuf, " id=\"%@", id);
          for(int i = bhl; i < lv; i++){
            tcxstrprintf(rbuf, "_%d", headcnts[i]);
          }
          tcxstrprintf(rbuf, "\"");
        }
        tcxstrprintf(rbuf, " class=\"ah%d topic\">", abslv);
        txtlen += wikitohtmlinline(rbuf, rp, buri, duri);
        tcxstrprintf(rbuf, "</h%d>\n", lv);
        for(int i = lv; i < HEADLVMAX; i++){
          headcnts[lv] = 0;
        }
      }
      ri++;
    } else if(*line == '-' || *line == '+'){
      int ei = ri + 1;
      while(ei < lnum){
        const char *rp = tclistval2(lines, ei);
        if(*rp != '-' && *rp != '+') break;
        ei++;
      }
      TCLIST *stack = tclistnew();
      for(int i = ri; i < ei; i++){
        const char *rp = tclistval2(lines, i);
        int sep = *rp;
        int clv = 1;
        rp++;
        while(*rp == sep){
          clv++;
          rp++;
        }
        rp = tcstrskipspc(rp);
        if(clv <= tclistnum(stack)) tcxstrcat2(rbuf, "</li>\n");
        while(clv < tclistnum(stack)){
          char *tag = tclistpop2(stack);
          tcxstrprintf(rbuf, "</%s>\n", tag);
          tcfree(tag);
          tcxstrcat2(rbuf, "</li>\n");
        }
        for(int k = 0; clv > tclistnum(stack); k++){
          if(k > 0) tcxstrcat2(rbuf, "<li>\n");
          const char *tag = (sep == '-') ? "ul" : "ol";
          tclistpush2(stack, tag);
          tcxstrprintf(rbuf, "<%s>\n", tag);
        }
        tcxstrcat2(rbuf, "<li>");
        txtlen += wikitohtmlinline(rbuf, rp, buri, duri);
      }
      while(tclistnum(stack) > 0){
        tcxstrcat2(rbuf, "</li>\n");
        char *tag = tclistpop2(stack);
        tcxstrprintf(rbuf, "</%s>\n", tag);
        tcfree(tag);
      }
      tclistdel(stack);
      ri = ei;
    } else if(*line == ',' || *line == '|'){
      int ei = ri + 1;
      while(ei < lnum){
        const char *rp = tclistval2(lines, ei);
        if(*rp != ',' && *rp != '|') break;
        ei++;
      }
      tcxstrprintf(rbuf, "<table summary=\"table:%d\">\n", ++tblcnt);
      for(int i = ri; i < ei; i++){
        const char *rp = tclistval2(lines, i);
        int sep = *rp;
        rp++;
        tcxstrcat2(rbuf, "<tr>\n");
        while(true){
          tcxstrcat2(rbuf, "<td>");
          const char *pv = strchr(rp, sep);
          char *field = pv ? tcmemdup(rp, pv - rp) : tcstrdup(rp);
          tcstrtrim(field);
          txtlen += wikitohtmlinline(rbuf, field, buri, duri);
          tcfree(field);
          tcxstrcat2(rbuf, "</td>\n");
          if(!pv) break;
          rp = pv + 1;
        }
        tcxstrcat2(rbuf, "</tr>\n");
      }
      tcxstrcat2(rbuf, "</table>\n");
      ri = ei;
    } else if(*line == '>'){
      int ei = ri + 1;
      while(ei < lnum){
        const char *rp = tclistval2(lines, ei);
        if(*rp != '>') break;
        ei++;
      }
      tcxstrcat2(rbuf, "<blockquote>\n");
      for(int i = ri; i < ei; i++){
        const char *rp = tclistval2(lines, i);
        rp = tcstrskipspc(rp + 1);
        if(*rp != '\0'){
          tcxstrcat2(rbuf, "<p>");
          txtlen += wikitohtmlinline(rbuf, rp, buri, duri);
          tcxstrcat2(rbuf, "</p>\n");
        }
      }
      tcxstrcat2(rbuf, "</blockquote>\n");
      ri = ei;
    } else if(tcstrfwm(line, "{{{")){
      TCXSTR *sep = tcxstrnew();
      line += 3;
      while(*line != '\0'){
        switch(*line){
          case '{': tcxstrprintf(sep, "%c", '}'); break;
          case '[': tcxstrprintf(sep, "%c", ']'); break;
          case '<': tcxstrprintf(sep, "%c", '>'); break;
          case '(': tcxstrprintf(sep, "%c", ')'); break;
          default: tcxstrcat(sep, line, 1); break;
        }
        line++;
      }
      tcxstrcat(sep, "}}}", 3);
      const char *sepstr = tcxstrptr(sep);
      ri++;
      int ei = ri;
      while(ei < lnum){
        const char *rp = tclistval2(lines, ei);
        if(!strcmp(rp, sepstr)) break;
        ei++;
      }
      tcxstrcat2(rbuf, "<pre>");
      for(int i = ri; i < ei; i++){
        const char *rp = tclistval2(lines, i);
        tcxstrprintf(rbuf, "%@\n", rp);
        txtlen += tcstrcntutf(rp);
      }
      tcxstrcat2(rbuf, "</pre>\n");
      tcxstrdel(sep);
      ri = ei + 1;
    } else if(*line == '@'){
      line++;
      imgcnt++;
      bool anc = false;
      if(*line == '@'){
        anc = true;
        line++;
      }
      bool obj = false;
      if(*line == '!'){
        obj = true;
        line++;
      }
      bool vdo = false;
      if(*line == '*') {
        vdo = true;
        line++;
      }
      const char *align = "normal";
      int lv = 3;
      if(*line == '<'){
        align = "left";
        line++;
        while(*line == '<' || *line == '>'){
          lv += (*line == '<') ? 1 : -1;
          line++;
        }
      } else if(*line == '>'){
        align = "right";
        line++;
        while(*line == '<' || *line == '>'){
          lv += (*line == '>') ? 1 : -1;
          line++;
        }
      } else if(*line == '+'){
        align = "center";
        line++;
        while(*line == '+' || *line == '-'){
          lv += (*line == '+') ? 1 : -1;
          line++;
        }
      } else if(*line == '|'){
        align = "table";
        line++;
        while(*line == '|'){
          lv++;
          line++;
        }
      }
      if(lv < 1) lv = 1;
      if(lv > IMAGELVMAX) lv = IMAGELVMAX;
      line = tcstrskipspc(line);
      if(*line != '\0'){
        char *uri = tcstrdup(line);
        int width = 0;
        bool wratio = false;
        int height = 0;
        bool hratio = false;
        const char *alt = NULL;
        TCLIST *params = NULL;
        char *sep = strchr(uri, '|');
        if(sep){
          *(sep++) = '\0';
          width = tcatoi(sep);
          while(*sep >= '0' && *sep <= '9'){
            sep++;
          }
          if(*sep == '%') wratio = true;
          sep = strchr(sep, '|');
          if(sep){
            sep++;
            height = tcatoi(sep);
            while(*sep >= '0' && *sep <= '9'){
              sep++;
            }
            if(*sep == '%') hratio = true;
            sep = strchr(sep, '|');
            if(sep){
              sep++;
              alt = sep;
              sep = strchr(sep, '|');
              if(sep){
                *(sep++) = '\0';
                params = tcstrsplit(sep, "|");
              }
            }
          }
        }
        const char *name;
        char *url;
        if(tcstrfwm(uri, "upfile:")){
          const char *rp = strstr(uri, ":") + 1;
          url = tcsprintf("%s/%@", duri ? duri : buri, rp);
          name = rp;
        } else {
          url = tcstrdup(uri);
          name = strrchr(uri, '/');
          name = name ? name + 1 : uri;
        }
        const char *scale = width > 0 ? "sized" : "ratio";
        tcxstrprintf(rbuf, "<div class=\"image image_%s image_%s%d image_%s\">",
                     align, align, lv, scale);
        if(anc) tcxstrprintf(rbuf, "<a href=\"%@\">", url);
        if(obj){
          if(objnum < absobjnum) {
            tcxstrprintf(rbuf, "<object data=\"%@\"", url);
            const char *type = mimetype(url);
            if(type) tcxstrprintf(rbuf, " type=\"%@\"", type);
            if(width > 0) tcxstrprintf(rbuf, " width=\"%d%@\"", width, wratio ? "%" : "");
            if(height > 0) tcxstrprintf(rbuf, " height=\"%d%@\"", height, hratio ? "%" : "");
            tcxstrprintf(rbuf, ">");
            if(params){
              int pnum = tclistnum(params) - 1;
              for(int i = 0; i < pnum; i += 2){
                tcxstrprintf(rbuf, "<param name=\"%@\" value=\"%@\" />",
                             tclistval2(params, i), tclistval2(params, i + 1));
              }
            }
            if(alt){
              tcxstrprintf(rbuf, "%@", alt);
            } else {
              tcxstrprintf(rbuf, "object:%d:%@", imgcnt, name);
            }
            tcxstrprintf(rbuf, "</object>");
            objnum++;
          }
        } else if(vdo){
          if(objnum < absobjnum) {
            tcxstrprintf(rbuf, "<video src=\"%@\"", url);
            if(width > 0) tcxstrprintf(rbuf, " width=\"%d%s\"", width, wratio ? "%" : "");
            if(height > 0) tcxstrprintf(rbuf, " height=\"%d%s\"", height, hratio ? "%" : "");
            tcxstrprintf(rbuf, " preload=\"metadata\"");
            tcxstrprintf(rbuf, " controls=\"controls\"");
            if(alt){
              tcxstrprintf(rbuf, " alt=\"%@\"", alt);
            } else {
              tcxstrprintf(rbuf, " alt=\"video:%d:%@\"", imgcnt, name);
            }
            tcxstrprintf(rbuf, " />");
            objnum++;
          }
        } else {
          if(objnum < absobjnum) {
            tcxstrprintf(rbuf, "<img src=\"%@\"", url);
            if(width > 0) tcxstrprintf(rbuf, " width=\"%d%s\"", width, wratio ? "%" : "");
            if(height > 0) tcxstrprintf(rbuf, " height=\"%d%s\"", height, hratio ? "%" : "");
            if(alt){
              tcxstrprintf(rbuf, " alt=\"%@\"", alt);
            } else {
              tcxstrprintf(rbuf, " alt=\"image:%d:%@\"", imgcnt, name);
            }
            tcxstrprintf(rbuf, " />");
            objnum++;
          }
        }
        if(anc) tcxstrprintf(rbuf, "</a>");
        tcxstrprintf(rbuf, "</div>\n");
        tcfree(url);
        if(params) tclistdel(params);
        tcfree(uri);
      }
      ri++;
    } else if(tcstrfwm(line, "===")){
      while(*line == '='){
        line++;
      }
      int lv = 0;
      while(*line == '#'){
        line++;
        lv++;
      }
      if(lv > SPACELVMAX) lv = SPACELVMAX;
      tcxstrprintf(rbuf, "<div class=\"rule rule_s%d\">"
                   "<span>----</span></div>\n", lv);
      ri++;
    } else {
      line = tcstrskipspc(line);
      if(*line != '\0'){
        tcxstrcat2(rbuf, "<p>");
        txtlen += wikitohtmlinline(rbuf, line, buri, duri);
        tcxstrcat2(rbuf, "</p>\n");
        ri++;
      } else {
        ri++;
      }
    }
  }
  tclistdel(lines);
  return txtlen;
}


/* Add an inline Wiki string into HTML. */
int wikitohtmlinline(TCXSTR *rbuf, const char *line, const char *buri, const char *duri){
  assert(rbuf && line && buri);
  int txtlen = 0;
  while(*line != '\0'){
    const char *pv;
    if(*line == '['){
      if(tcstrfwm(line, "[[") && (pv = strstr(line + 2, "]]")) != NULL){
        char *field = tcmemdup(line + 2, pv - line - 2);
        const char *uri = field;
        char *sep = strchr(field, '|');
        if(sep){
          *sep = '\0';
          uri = sep + 1;
        }
        if(tcstrfwm(uri, "http://") || tcstrfwm(uri, "https://") ||
           tcstrfwm(uri, "ftp://") || tcstrfwm(uri, "mailto:")){
          tcxstrprintf(rbuf, "<a href=\"%@\">", uri);
        } else if(tcstrfwm(uri, "id:")){
          int64_t id = tcatoi(tcstrskipspc(strchr(uri, ':') + 1));
          tcxstrprintf(rbuf, "<a href=\"%s?id=%lld\">", buri, (long long)(id > 0 ? id : 0));
        } else if(tcstrfwm(uri, "name:")){
          uri = tcstrskipspc(strchr(uri, ':') + 1);
          tcxstrprintf(rbuf, "<a href=\"%s?name=%?\">", buri, uri);
        } else if(tcstrfwm(uri, "param:")){
          uri = tcstrskipspc(strchr(uri, ':') + 1);
          tcxstrprintf(rbuf, "<a href=\"%s%s%@\">", buri, *uri != 0 ? "?" : "", uri);
        } else if(tcstrfwm(uri, "upfile:")){
          uri = tcstrskipspc(strchr(uri, ':') + 1);
          tcxstrprintf(rbuf, "<a href=\"%s/%@\">", duri ? duri : buri, uri);
        } else if(tcstrfwm(uri, "wpen:")){
          uri = tcstrskipspc(strchr(uri, ':') + 1);
          if(*uri == '\0') uri = field;
          tcxstrprintf(rbuf, "<a href=\"http://en.wikipedia.org/wiki/%?\">", uri);
        } else if(tcstrfwm(uri, "wpja:")){
          uri = tcstrskipspc(strchr(uri, ':') + 1);
          if(*uri == '\0') uri = field;
          tcxstrprintf(rbuf, "<a href=\"http://ja.wikipedia.org/wiki/%?\">", uri);
        } else if(sep){
          uri = tcstrskipspc(uri);
          tcxstrprintf(rbuf, "<a href=\"%@\">", uri);
        } else {
          uri = tcstrskipspc(uri);
          tcxstrprintf(rbuf, "<a href=\"%s?name=%?\">", buri, uri);
        }
        txtlen += wikitohtmlinline(rbuf, field, buri, duri);
        tcxstrprintf(rbuf, "</a>");
        tcfree(field);
        line = pv + 2;
      } else if(tcstrfwm(line, "[*") && (pv = strstr(line + 2, "*]")) != NULL){
        char *field = tcmemdup(line + 2, pv - line - 2);
        tcxstrcat2(rbuf, "<strong>");
        txtlen += wikitohtmlinline(rbuf, field, buri, duri);
        tcxstrcat2(rbuf, "</strong>");
        tcfree(field);
        line = pv + 2;
      } else if(tcstrfwm(line, "[\"") && (pv = strstr(line + 2, "\"]")) != NULL){
        char *field = tcmemdup(line + 2, pv - line - 2);
        tcxstrcat2(rbuf, "<cite>");
        txtlen += wikitohtmlinline(rbuf, field, buri, duri);
        tcxstrcat2(rbuf, "</cite>");
        tcfree(field);
        line = pv + 2;
      } else if(tcstrfwm(line, "[+") && (pv = strstr(line + 2, "+]")) != NULL){
        char *field = tcmemdup(line + 2, pv - line - 2);
        tcxstrcat2(rbuf, "<ins>");
        txtlen += wikitohtmlinline(rbuf, field, buri, duri);
        tcxstrcat2(rbuf, "</ins>");
        tcfree(field);
        line = pv + 2;
      } else if(tcstrfwm(line, "[-") && (pv = strstr(line + 2, "-]")) != NULL){
        char *field = tcmemdup(line + 2, pv - line - 2);
        tcxstrcat2(rbuf, "<del>");
        txtlen += wikitohtmlinline(rbuf, field, buri, duri);
        tcxstrcat2(rbuf, "</del>");
        tcfree(field);
        line = pv + 2;
      } else if(tcstrfwm(line, "[#") && (pv = strstr(line + 2, "#]")) != NULL){
        char *field = tcmemdup(line + 2, pv - line - 2);
        tcxstrcat2(rbuf, "<code>");
        txtlen += wikitohtmlinline(rbuf, field, buri, duri);
        tcxstrcat2(rbuf, "</code>");
        tcfree(field);
        line = pv + 2;
      } else if(tcstrfwm(line, "[$") && (pv = strstr(line + 2, "$]")) != NULL){
        char *field = tcmemdup(line + 2, pv - line - 2);
        tcxstrcat2(rbuf, "<var>");
        txtlen += wikitohtmlinline(rbuf, field, buri, duri);
        tcxstrcat2(rbuf, "</var>");
        tcfree(field);
        line = pv + 2;
      } else if(tcstrfwm(line, "[=") && (pv = strstr(line + 2, "=]")) != NULL){
        char *field = tcmemdup(line + 2, pv - line - 2);
        tcxstrprintf(rbuf, "%@", field);
        txtlen += tcstrcntutf(field);
        tcfree(field);
        line = pv + 2;
      } else {
        switch(*line){
          case '&': tcxstrcat(rbuf, "&amp;", 5); break;
          case '<': tcxstrcat(rbuf, "&lt;", 4); break;
          case '>': tcxstrcat(rbuf, "&gt;", 4); break;
          case '"': tcxstrcat(rbuf, "&quot;", 6); break;
          default: tcxstrcat(rbuf, line, 1); break;
        }
        if((*line & 0x80) == 0x00 || (*line & 0xe0) == 0xc0 ||
           (*line & 0xf0) == 0xe0 || (*line & 0xf8) == 0xf0) txtlen++;
        line++;
      }
    } else {
      switch(*line){
        case '&': tcxstrcat(rbuf, "&amp;", 5); break;
        case '<': tcxstrcat(rbuf, "&lt;", 4); break;
        case '>': tcxstrcat(rbuf, "&gt;", 4); break;
        case '"': tcxstrcat(rbuf, "&quot;", 6); break;
        default: tcxstrcat(rbuf, line, 1); break;
      }
      if((*line & 0x80) == 0x00 || (*line & 0xe0) == 0xc0 ||
         (*line & 0xf0) == 0xe0 || (*line & 0xf8) == 0xf0) txtlen++;
      line++;
    }
  }
  return txtlen;
}


/* Simplify a date string. */
char *datestrsimple(char *str){
  assert(str);
  char *pv = str;
  while(*pv > '\0' && *pv <= ' '){
    pv++;
  }
  int len = strlen(pv);
  if(len <= 16 || pv[4] != '-' || pv[7] != '-' || pv[10] != 'T' || pv[13] != ':') return pv;
  pv[4] = '/';
  pv[7] = '/';
  pv[10] = ' ';
  pv[16] = '\0';
  return str;
}


/* Get the MIME type of a file. */
const char *mimetype(const char *name){
  assert(name);
  const char *list[] = {
    "txt", "text/plain", "text", "text/plain", "asc", "text/plain", "in", "text/plain",
    "c", "text/plain", "h", "text/plain", "cc", "text/plain", "java", "text/plain",
    "sh", "text/plain", "pl", "text/plain", "py", "text/plain", "rb", "text/plain",
    "idl", "text/plain", "csv", "text/plain", "log", "text/plain", "conf", "text/plain",
    "rc", "text/plain", "ini", "text/plain",
    "html", "text/html", "htm", "text/html", "xhtml", "text/html", "xht", "text/html",
    "css", "text/css", "js", "text/javascript", "tsv", "text/tab-separated-values",
    "eml", "message/rfc822", "mime", "message/rfc822", "mht", "message/rfc822",
    "mhtml", "message/rfc822", "sgml", "application/sgml",
    "sgm", "application/sgml", "xml", "application/xml", "xsl", "application/xml",
    "xslt", "application/xslt+xml", "xhtml", "application/xhtml+xml",
    "xht", "application/xhtml+xml", "rdf", "application/rdf+xml",
    "rss", "application/rss+xml", "dtd", "application/xml-dtd",
    "rtf", "application/rtf", "pdf", "application/pdf",
    "ps", "application/postscript", "eps", "application/postscript",
    "doc", "application/msword", "xls", "application/vnd.ms-excel",
    "ppt", "application/vnd.ms-powerpoint", "xdw", "application/vnd.fujixerox.docuworks",
    "swf", "application/x-shockwave-flash", "zip", "application/zip",
    "tar", "application/x-tar", "gz", "application/x-gzip",
    "bz2", "application/octet-stream", "z", "application/octet-stream",
    "lha", "application/octet-stream", "lzh", "application/octet-stream",
    "cab", "application/octet-stream", "rar", "application/octet-stream",
    "sit", "application/octet-stream", "bin", "application/octet-stream",
    "o", "application/octet-stream", "so", "application/octet-stream",
    "exe", "application/octet-stream", "dll", "application/octet-stream",
    "class", "application/octet-stream",
    "tch", "application/x-tokyocabinet-hash", "tcb", "application/x-tokyocabinet-btree",
    "tcf", "application/x-tokyocabinet-fixed", "tct", "application/x-tokyocabinet-table",
    "png", "image/png", "jpg", "image/jpeg", "jpeg", "image/jpeg",
    "gif", "image/gif", "tif", "image/tiff", "tiff", "image/tiff", "bmp", "image/bmp",
    "svg", "image/svg+xml", "xbm", "image/x-xbitmap",
    "au", "audio/basic", "snd", "audio/basic", "mid", "audio/midi", "midi", "audio/midi",
    "mp2", "audio/mpeg", "mp3", "audio/mpeg", "wav", "audio/x-wav", "mpg", "video/mpeg",
    "mpeg", "video/mpeg", "mp4", "video/mp4", "qt", "video/quicktime",
    "mov", "video/quicktime", "avi", "video/x-msvideo",
    NULL
  };
  int len = strlen(name);
  char stack[1024];
  char *buf = NULL;
  if(len < sizeof(stack)){
    buf = stack;
  } else {
    buf = tcmalloc(len + 1);
  }
  memcpy(buf, name, len);
  buf[len] = '\0';
  char *pv = strchr(buf, '#');
  if(pv) *pv = '\0';
  pv = strchr(buf, '?');
  if(pv) *pv = '\0';
  const char *ext = strrchr(buf, '.');
  const char *type = NULL;
  if(ext){
    ext++;
    for(int i = 0; list[i] != NULL; i++){
      if(!tcstricmp(ext, list[i])){
        type = list[i+1];
        break;
      }
    }
  }
  if(buf != stack) tcfree(buf);
  return type;
}


/* Get the human readable name of the MIME type. */
const char *mimetypename(const char *type){
  const char *rp = strchr(type, '/');
  if(!rp) return "unknown";
  rp++;
  if(tcstrfwm(type, "text/")){
    if(!strcmp(rp, "plain")) return "plain text";
    if(!strcmp(rp, "html")) return "HTML text";
    if(!strcmp(rp, "css")) return "CSS text";
    if(!strcmp(rp, "javascript")) return "JavaScript";
    if(!strcmp(rp, "tab-separated-values")) return "TSV text";
    return "text";
  }
  if(tcstrfwm(type, "message/")){
    if(!strcmp(rp, "rfc822")) return "MIME";
    return "message";
  }
  if(tcstrfwm(type, "application/")){
    if(!strcmp(rp, "sgml")) return "SGML";
    if(!strcmp(rp, "xml")) return "XML";
    if(!strcmp(rp, "xslt+xml")) return "XSLT";
    if(!strcmp(rp, "xhtml+xml")) return "XHTML";
    if(!strcmp(rp, "rdf+xml")) return "RDF";
    if(!strcmp(rp, "rss+xml")) return "RSS";
    if(!strcmp(rp, "xml-dtd")) return "DTD";
    if(!strcmp(rp, "rtf")) return "RTF";
    if(!strcmp(rp, "pdf")) return "PDF";
    if(!strcmp(rp, "postscript")) return "PostScript";
    if(!strcmp(rp, "msword")) return "MS-Word";
    if(!strcmp(rp, "vnd.ms-excel")) return "MS-Excel";
    if(!strcmp(rp, "vnd.ms-powerpoint")) return "MS-PowerPoint";
    if(!strcmp(rp, "vnd.fujixerox.docuworks")) return "FX-DocuWorks";
    if(!strcmp(rp, "x-shockwave-flash")) return "Flash";
    return "binary";
  }
  if(tcstrfwm(type, "image/")){
    if(!strcmp(rp, "png")) return "PNG image";
    if(!strcmp(rp, "jpeg")) return "JPEG image";
    if(!strcmp(rp, "gif")) return "GIF image";
    if(!strcmp(rp, "tiff")) return "TIFF image";
    if(!strcmp(rp, "bmp")) return "BMP image";
    if(!strcmp(rp, "svg+xml")) return "SVG image";
    if(!strcmp(rp, "x-xbitmap")) return "XBM image";
    return "image";
  }
  if(tcstrfwm(type, "audio/")){
    if(!strcmp(rp, "basic")) return "basic audio";
    if(!strcmp(rp, "midi")) return "MIDI audio";
    if(!strcmp(rp, "mpeg")) return "MPEG audio";
    if(!strcmp(rp, "x-wav")) return "WAV audio";
    return "audio";
  }
  if(tcstrfwm(type, "video/")){
    if(!strcmp(rp, "mpeg")) return "MPEG video";
    if(!strcmp(rp, "quicktime")) return "QuickTime";
    if(!strcmp(rp, "x-msvideo")) return "AVI video";
    return "video";
  }
  return "other";
}


/* Encode a string with file path encoding. */
char *pathencode(const char *str){
  assert(str);
  int len = strlen(str);
  char *res = tcmalloc(len * 3 + 1);
  char *wp = res;
  while(true){
    int c = *(unsigned char *)str;
    if(c == '\0'){
      break;
    } else if(c == ' '){
      *(wp++) = '+';
    } else if(c < ' ' || c == '%' || c == '/' || c == 0x7f){
      wp += sprintf(wp, "%%%02X", c);
    } else {
      *(wp++) = c;
    }
    str++;
  }
  *wp = '\0';
  return res;
}


/* Trim invalid characters of an XML string. */
char *trimxmlchars(char *str) {
  assert(str);
  const unsigned char *rp = (unsigned char *)str;
  unsigned char *wp = (unsigned char *)str;
  while (*rp != '\0') {
    int c = 0;
    int len = 0;
    if(*rp < 0x80){
      c = *rp;
      len = 1;
    } else if(*rp < 0xe0){
      if(rp[1] >= 0x80){
        c = ((rp[0] & 0x1f) << 6) | (rp[1] & 0x3f);
        len = 2;
      }
    } else if(*rp < 0xf0){
      if(rp[1] >= 0x80 && rp[2] >= 0x80){
        c = ((rp[0] & 0xf) << 12) | ((rp[1] & 0x3f) << 6) | (rp[2] & 0x3f);
        len = 3;
      }
    }
    if(len < 1) break;
    if(c == 0x0A || c == 0x0D || (c >= 0x20 && c <= 0xD7FF) || (c >= 0xE000 && c <= 0xFFFD) ||
       (c >= 0x10000 && c <= 0x10FFFF)) {
      while (len-- > 0) {
        *(wp++) = *(rp++);
      }
    } else {
      *(wp++) = '?';
      rp += len;
    }
  }
  *wp = '\0';
  return str;
}


/* Store an article into a database. */
bool dbputart(TCTDB *tdb, int64_t id, TCMAP *cols){
  assert(tdb && cols);
  if(id < 1){
    id = tctdbgenuid(tdb);
    if(id < 1) return false;
  }
  const char *name = tcmapget2(cols, "name");
  if(!name || *name == '\0'){
    tctdbsetecode(tdb, TCEINVALID, __FILE__, __LINE__, __func__);
    return false;
  }
  bool err = false;
  tcmapout2(cols, "id");
  int msiz = tcmapmsiz(cols);
  TCXSTR *wiki = tcxstrnew3(msiz + 1);
  wikidump(wiki, cols);
  TCMAP *ncols = tcmapnew2(TINYBNUM);
  wikiload(ncols, tcxstrptr(wiki));
  char pkbuf[NUMBUFSIZ];
  int pksiz = sprintf(pkbuf, "%lld", (long long)id);
  if(tctdbtranbegin(tdb)){
    if(tctdbput(tdb, pkbuf, pksiz, ncols)){
      if(tctdbtrancommit(tdb)){
        tcmapput2(cols, "id", pkbuf);
      } else {
        err = true;
      }
    } else {
      err = true;
      tctdbtranabort(tdb);
    }
  } else {
    err = true;
  }
  tcmapdel(ncols);
  tcxstrdel(wiki);
  return !err;
}


/* Remove an article from the database. */
bool dboutart(TCTDB *tdb, int64_t id){
  assert(tdb && id > 0);
  bool err = false;
  char pkbuf[NUMBUFSIZ];
  int pksiz = sprintf(pkbuf, "%lld", (long long)id);
  if(tctdbtranbegin(tdb)){
    if(tctdbout(tdb, pkbuf, pksiz)){
      if(!tctdbtrancommit(tdb)) err = true;
    } else {
      err = true;
      tctdbtranabort(tdb);
    }
  } else {
    err = true;
  }
  return !err;
}


/* Retrieve an article of the database. */
TCMAP *dbgetart(TCTDB *tdb, int64_t id){
  assert(tdb && id > 0);
  char pkbuf[NUMBUFSIZ];
  int pksiz = sprintf(pkbuf, "%lld", (long long)id);
  return tctdbget(tdb, pkbuf, pksiz);
}


/* Generate the hash value of a user password. */
void passwordhash(const char *pass, const char *salt, char *buf){
  assert(pass && salt && buf);
  TCXSTR *xstr = tcxstrnew();
  if(*salt != 0) tcxstrprintf(xstr, "%s:", salt);
  tcxstrcat2(xstr, pass);
  tcmd5hash(tcxstrptr(xstr), tcxstrsize(xstr), buf);
  tcxstrdel(xstr);
}


/* Check whether the name of a user is valid. */
bool checkusername(const char *name){
  assert(name);
  if(*name == '\0') return false;
  while(true){
    int c = *(name++);
    if(c == '\0') return true;
    if(c >= 'a' && c <= 'z') continue;
    if(c >= 'A' && c <= 'Z') continue;
    if(c >= '0' && c <= '9') continue;
    if(strchr("_-.", c)) continue;
    break;
  }
  return false;
}


/* Check whether an article is frozen. */
bool checkfrozen(TCMAP *cols){
  assert(cols);
  const char *tags = tcmapget2(cols, "tags");
  if(!tags) return false;
  for(const char *rp = tags; *rp != '\0'; rp++){
    if((*rp == '*') && (rp == tags || rp[-1] == ' ' || rp[-1] == ',') &&
       (rp[1] == '\0' || rp[1] == ' ' || rp[1] == ',')) return true;
  }
  return false;
}



// END OF FILE
