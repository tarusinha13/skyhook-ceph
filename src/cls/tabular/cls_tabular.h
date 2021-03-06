/*
* Copyright (C) 2018 The Regents of the University of California
* All Rights Reserved
*
* This library can redistribute it and/or modify under the terms
* of the GNU Lesser General Public License Version 2.1 as published
* by the Free Software Foundation.
*
*/


#ifndef CLS_TABULAR_H
#define CLS_TABULAR_H

#include "include/types.h"


void cls_log_message(std::string msg, bool is_err, int log_level);

/*
 * Stores the query request parameters.  This is encoded by the client and
 * decoded by server (osd node) for query processing.
 */
struct query_op {

  // query parameters (old)
  std::string query;   // query name
  double extended_price;
  int order_key;
  int line_number;
  int ship_date_low;
  int ship_date_high;
  double discount_low;
  double discount_high;
  double quantity;
  std::string comment_regex;
  bool use_index;
  bool projection;
  uint64_t extra_row_cost;

  // query parameters (new) flatbufs
  bool fastpath;
  bool index_read;
  int index_type;
  int index2_type;
  int index_plan_type;
  std::string db_schema;
  std::string table_name;
  std::string data_schema;
  std::string query_schema;
  std::string index_schema;
  std::string index2_schema;
  std::string query_preds;
  std::string index_preds;
  std::string index2_preds;

  query_op() {}

  // serialize the fields into bufferlist to be sent over the wire
  void encode(bufferlist& bl) const {
    ENCODE_START(1, 1, bl);
    ::encode(query, bl);
    ::encode(extended_price, bl);
    ::encode(order_key, bl);
    ::encode(line_number, bl);
    ::encode(ship_date_low, bl);
    ::encode(ship_date_high, bl);
    ::encode(discount_low, bl);
    ::encode(discount_high, bl);
    ::encode(quantity, bl);
    ::encode(comment_regex, bl);
    ::encode(use_index, bl);
    ::encode(projection, bl);
    ::encode(extra_row_cost, bl);
    // flatbufs
    ::encode(fastpath, bl);
    ::encode(index_read, bl);
    ::encode(index_type, bl);
    ::encode(index2_type, bl);
    ::encode(index_plan_type, bl);
    ::encode(db_schema, bl);
    ::encode(table_name, bl);
    ::encode(data_schema, bl);
    ::encode(query_schema, bl);
    ::encode(index_schema, bl);
    ::encode(index2_schema, bl);
    ::encode(query_preds, bl);
    ::encode(index_preds, bl);
    ::encode(index2_preds, bl);
    ENCODE_FINISH(bl);
  }

  // deserialize the fields from the bufferlist into this struct
  void decode(bufferlist::iterator& bl) {
    DECODE_START(1, bl);
    ::decode(query, bl);
    ::decode(extended_price, bl);
    ::decode(order_key, bl);
    ::decode(line_number, bl);
    ::decode(ship_date_low, bl);
    ::decode(ship_date_high, bl);
    ::decode(discount_low, bl);
    ::decode(discount_high, bl);
    ::decode(quantity, bl);
    ::decode(comment_regex, bl);
    ::decode(use_index, bl);
    ::decode(projection, bl);
    ::decode(extra_row_cost, bl);
    // flatbufs
    ::decode(fastpath, bl);
    ::decode(index_read, bl);
    ::decode(index_type, bl);
    ::decode(index2_type, bl);
    ::decode(index_plan_type, bl);
    ::decode(db_schema, bl);
    ::decode(table_name, bl);
    ::decode(data_schema, bl);
    ::decode(query_schema, bl);
    ::decode(index_schema, bl);
    ::decode(index2_schema, bl);
    ::decode(query_preds, bl);
    ::decode(index_preds, bl);
    ::decode(index2_preds, bl);
    DECODE_FINISH(bl);
  }

  std::string toString() {
    std::string s;
    s.append("op:");
    s.append(" .fastpath=" + std::to_string(fastpath));
    s.append(" .index_read=" + std::to_string(index_read));
    s.append(" .index_type=" + std::to_string(index_type));
    s.append(" .index2_type=" + std::to_string(index2_type));
    s.append(" .index_plan_type=" + std::to_string(index_plan_type));
    s.append(" .db_schema=" + db_schema);
    s.append(" .table_name=" + table_name);
    s.append(" .data_schema=" + data_schema);
    s.append(" .query_schema=" + query_schema);
    s.append(" .index_schema=" + index_schema);
    s.append(" .index2_schema=" + index2_schema);
    s.append(" .query_preds=" + query_preds);
    s.append(" .index_preds=" + index_preds);
    s.append(" .index2_preds=" + index2_preds);
    return s;
  }
};
WRITE_CLASS_ENCODER(query_op)

// holds an omap entry containing flatbuffer location
// this entry type contains physical location info
// idx_key = idx_prefix + fb sequence number (int)
// val = this struct containing to PHYSICAL location of fb within obj
// note: objs contain a sequence of fbs, hence the off/len is needed
struct idx_fb_entry {
    uint32_t off;
    uint32_t len;

    idx_fb_entry() {}
    idx_fb_entry(uint32_t o, uint32_t l) : off(o), len(l) { }

    void encode(bufferlist& bl) const {
        ENCODE_START(1, 1, bl);
        ::encode(off, bl);
        ::encode(len, bl);
        ENCODE_FINISH(bl);
    }

    void decode(bufferlist::iterator& bl) {
        DECODE_START(1, bl);
        ::decode(off, bl);
        ::decode(len, bl);
        DECODE_FINISH(bl);
    }

    std::string toString() {
        std::string s;
        s.append("idx_fb_entry.off=" + std::to_string(off));
        s.append("; idx_fb_entry.len=" + std::to_string(len));
        return s;
    }
};
WRITE_CLASS_ENCODER(idx_fb_entry)

// holds an omap entry for indexed col values
// this index entry type contains logical location info
// idx_key = idx_prefix + column data value(s) (ints)
// val = this struct containing to LOGICAL location of row within an fb
struct idx_rec_entry {
    uint32_t fb_num;
    uint32_t row_num;
    uint64_t rid;

    idx_rec_entry() {}
    idx_rec_entry(uint32_t fb, uint32_t row, uint64_t rec_id) :
            fb_num(fb),
            row_num(row),
            rid(rec_id){}

    void encode(bufferlist& bl) const {
        ENCODE_START(1, 1, bl);
        ::encode(fb_num, bl);
        ::encode(row_num, bl);
        ::encode(rid, bl);
        ENCODE_FINISH(bl);
    }

    void decode(bufferlist::iterator& bl) {
        DECODE_START(1, bl);
        ::decode(fb_num, bl);
        ::decode(row_num, bl);
        ::decode(rid, bl);
        DECODE_FINISH(bl);
    }

    std::string toString() {
        std::string s;
        s.append("idx_rec_entry.fb_num=" + std::to_string(fb_num));
        s.append("; idx_rec_entry.row_num=" + std::to_string(row_num));
        s.append("; idx_rec_entry.rid=" + std::to_string(rid));
        return s;
    }
};
WRITE_CLASS_ENCODER(idx_rec_entry)

// omap entry for text index
// this index entry type contains logical location info
// idx_key = idx_prefix + column data value (here a text string)
// val = this struct containing to logical location of row within fb
struct idx_txt_entry {
    uint32_t fb_num;
    uint32_t row_num;
    uint64_t rid;
    uint32_t wpos;  // word's relative position in column, i.e., word number

    idx_txt_entry() {}
    idx_txt_entry(uint32_t fb, uint32_t row, uint64_t rec_id, uint32_t wp):
            fb_num(fb),
            row_num(row),
            rid(rec_id),
            wpos(wp) {}

    void encode(bufferlist& bl) const {
        ENCODE_START(1, 1, bl);
        ::encode(fb_num, bl);
        ::encode(row_num, bl);
        ::encode(rid, bl);
        ::encode(wpos, bl);
        ENCODE_FINISH(bl);
    }

    void decode(bufferlist::iterator& bl) {
        DECODE_START(1, bl);
        ::decode(fb_num, bl);
        ::decode(row_num, bl);
        ::decode(rid, bl);
        ::decode(wpos, bl);
        DECODE_FINISH(bl);
    }

    std::string toString() {
        std::string s;
        s.append("idx_txt_entry.fb_num=" + std::to_string(fb_num));
        s.append("; idx_txt_entry.row_num=" + std::to_string(row_num));
        s.append("; idx_txt_entry.rid=" + std::to_string(rid));
        s.append("; idx_txt_entry.wpos=" + std::to_string(wpos));
        return s;
    }
};
WRITE_CLASS_ENCODER(idx_txt_entry)

// Stores index instructions/metadata into bl for build_sky_index()
struct idx_op {
    bool idx_unique;   // if idx contains all primary key/unique cols
    bool idx_ignore_stopwords;  // for text indexing
    uint32_t idx_batch_size;  // num idx entries to write into omap at once
    int idx_type;
    std::string idx_schema_str;
    std::string idx_delims; // for text indexing

    idx_op() {}
    idx_op(bool unq, bool ign, int batsz, int index_type,
           std::string schema_str, std::string delimiters) :
        idx_unique(unq),
        idx_ignore_stopwords(ign),
        idx_batch_size(batsz),
        idx_type(index_type),
        idx_schema_str(schema_str),
        idx_delims(delimiters) {}

    void encode(bufferlist& bl) const {
        ENCODE_START(1, 1, bl);
        ::encode(idx_unique, bl);
        ::encode(idx_ignore_stopwords, bl);
        ::encode(idx_batch_size, bl);
        ::encode(idx_type, bl);
        ::encode(idx_schema_str, bl);
        ::encode(idx_delims, bl);
        ENCODE_FINISH(bl);
    }

    void decode(bufferlist::iterator& bl) {
        std::string s;
        DECODE_START(1, bl);
        ::decode(idx_unique, bl);
        ::decode(idx_ignore_stopwords, bl);
        ::decode(idx_batch_size, bl);
        ::decode(idx_type, bl);
        ::decode(idx_schema_str, bl);
        ::decode(idx_delims, bl);
        DECODE_FINISH(bl);
    }

    std::string toString() {
        std::string s;
        s.append("idx_op.idx_unique=" + std::to_string(idx_unique));
        s.append("idx_op.idx_ignore_stopwords=" +
                 std::to_string(idx_ignore_stopwords));
        s.append("; idx_op.idx_batch_size=" + std::to_string(idx_batch_size));
        s.append("; idx_op.idx_type=" + std::to_string(idx_type));
        s.append("; idx_op.idx_schema_str=\n" + idx_schema_str);
        s.append("; idx_op.delims=\n" + idx_delims);
        return s;
    }
};
WRITE_CLASS_ENCODER(idx_op)

#endif
