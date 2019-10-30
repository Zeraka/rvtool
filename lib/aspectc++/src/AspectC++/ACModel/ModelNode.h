#ifndef __ModelNode_h__
#define __ModelNode_h__

#include "MatchExpr.h"

#include <iostream>
using std::cout;
using std::endl;
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <string>
using std::string;
#include <list>
#include <set>
#include <map>

class ModelTransformInfo {
public:
  virtual ~ModelTransformInfo () {}
};

class SaveContext {
  int _stage;
  int _last_id;
public:
  SaveContext () : _last_id(0) {
    static int stage = 0;
    _stage = stage++;
  }
  int next_id () { return _last_id++; }
  int stage ()  { return _stage; }
};

inline std::ostream &operator << (std::ostream &os, const MatchSignature &ms) {
  os << (ms.is_new() ? "<unused>" : "<used>");
  return os;
}

class ACM_Name;
typedef std::map<string, ACM_Name*> NameMap;
typedef NameMap::value_type NameMapPair;
inline std::ostream &operator << (std::ostream &os, const NameMap &nm) {
  os << "\"name map\"";
  return os;
}

class PointcutExpressionNode {
  int _ref_count;
public:
  PointcutExpressionNode () : _ref_count (1) {}
  virtual ~PointcutExpressionNode () {}

  // get/modify the reference count
  int get_ref () const { return _ref_count; }
  void ref(int delta = 1) { _ref_count += delta; }

  // set and get the corresponding syntax tree and expression child nodes
  virtual int args () const = 0;
  virtual PointcutExpressionNode *arg (int) const = 0;

  // duplicate the whole pointcut expression tree -- at least logically
  virtual PointcutExpressionNode* duplicate () = 0;
};

class PointcutExpression {
  PointcutExpressionNode *_node;
public:
  PointcutExpression () : _node (0) {}
  PointcutExpression (const PointcutExpression &to_copy) {
    _node = to_copy.get();
    if (_node != 0)
      _node = _node->duplicate();
  }
  PointcutExpression &operator = (const PointcutExpression &to_copy) {
    destroy(_node);
    _node = to_copy.get();
    if (_node != 0)
      _node = _node->duplicate();
    return *this;
  }
  ~PointcutExpression () { destroy (_node); }

  // destroy a pointcut expression tree
  void set (PointcutExpressionNode *expr) { _node = expr; }
  PointcutExpressionNode *get () const { return _node; }

  static void destroy (PointcutExpressionNode *expr) {
    if (!expr) return;
    expr->ref(-1);
    if (expr->get_ref () == 0) {
      for (int i = 0; i < expr->args (); i++)
        destroy (expr->arg (i));
      delete expr;
    }
  }
};

inline std::ostream &operator << (std::ostream &os, const PointcutExpression &e) {
  os << "parsed pointcut expression";
  return os;
}

class Code {
  string text_;
public:
  operator string () const { return text_; }
  void set (const string &s) { text_ = s; }
};

inline std::ostream &operator << (std::ostream &os, const Code &u) {
  os << "\"" << string (u) << "\"";
  return os;
}

// common base class of all join point model node types
class ModelNode {
  xmlNodePtr _xml_node;
  bool _xml_dirty;
  ModelNode *_parent;
  ModelNode *_partner;
  int _stage;
  int _id;
  /// parser-provided info for code transformation
  ModelTransformInfo *_trans;
  /// key for sorting this node
  mutable string _key;
  mutable bool _key_dirty;
  bool _deleted;
  int _merge_count;
  
  string &get_key () const {
    if (_key_dirty) {
      _key = key();
      _key_dirty = false;
    }
    return _key;
  }

  public:
    ModelNode() : _xml_node(0), _xml_dirty(true), _parent(0), _partner(0),
        _stage (-1), _id (-1), _trans(0), _key_dirty (true),
        _deleted (false), _merge_count (0) {}
  virtual ~ModelNode () {}

  bool is_deleted () const { return _deleted; }
  void is_deleted (bool d) { _deleted = d; }

  void set_merge_count (int mc) { _merge_count = mc; }
  int get_merge_count () {
    int pmc = (_parent ? _parent->get_merge_count() : 0);
    if (pmc > _merge_count && _parent->is_deleted()) {
      _merge_count = pmc;
      _deleted = true;
      _xml_node = 0;
      _xml_dirty = true;
    }
    return _merge_count;
  }

  bool operator == (const ModelNode& that) const { return !(*this < that || that < *this); }
  bool operator != (const ModelNode& that) const { return !(*this == that); }
  bool operator < (const ModelNode& that) const { return get_key() < that.get_key(); }

  virtual string key() const = 0;

  ModelTransformInfo *transform_info () const { return _trans; }
  void transform_info (ModelTransformInfo *trans) { _trans = trans; }

  // set and get XML node pointer
  xmlNodePtr get_xml_node () const { return _xml_node; }
  void set_xml_node (xmlNodePtr n) { _xml_node = n; }

  // remember if the associated XML node is up-to-date
  bool get_xml_dirty () const { return _xml_dirty; }
  void set_xml_dirty (bool d = true) { _xml_dirty = d; if (d) _key_dirty = true; }

  // set and get the parent node
  void set_parent (ModelNode *n) { _parent = n; }
  ModelNode *get_parent () const { return _parent; }

  // update the XML representation of this object. Attributes might have
  // changed or the node is new.
  virtual void to_xml(SaveContext &) = 0;
  
  // set a string property
  void set_property(const string &prop, const string &val) {
    xmlSetProp (_xml_node, (xmlChar*)prop.c_str (), (xmlChar*)val.c_str ());
  }
  // set an int property
  void set_property(const string &prop, int val) {
  char buffer[12];
  sprintf (buffer, "%d", val);
    xmlSetProp (_xml_node, (xmlChar*)prop.c_str (), (xmlChar*)buffer);
  }
  // set a bool property
  void set_property(const string &prop, bool val) {
    if (val)
      set_property (prop, string ("true"));
    else
      remove_property (prop);
  }

  // store string as text child node
  void set_text_child(const string &childname, const string &text) {
    xmlNewTextChild (_xml_node, NULL, (xmlChar*)childname.c_str (),
        (xmlChar*)text.c_str ());
  }
  // check if the XML node has a specific property
  bool has_property (const string &prop) {
    return xmlHasProp (_xml_node, (xmlChar*)prop.c_str ()) != NULL;
  }
  
  // get a string property
  void get_property (const string &prop, string &val) {
    char *str = (char*)xmlGetProp (_xml_node, (xmlChar*)prop.c_str ());
    val = str;
    free (str);
  }
  
  // get an int property
  void get_property (const string &prop, int &val) {
    char *str = (char*)xmlGetProp (_xml_node, (xmlChar*)prop.c_str ());
    val = atoi (str);
    free (str);
  }
  
  // delete a property
  bool remove_property (const string &prop) {
    xmlAttrPtr attr = xmlHasProp (_xml_node, (xmlChar*)prop.c_str ());
    if (attr)
      xmlRemoveProp (attr);
    return attr != NULL;
  }

  // set and get the element's id, which is used by idrefs of other elements
  int get_id () const { return _id; }
  int assign_id (SaveContext &context) {
    if (_stage != context.stage ()) {
      _stage = context.stage ();
      _id = context.next_id ();
      if (_xml_node)
        set_property ("id", _id);
    }
    return _id;
  }

  ModelNode *get_partner () const { return _partner; }
  void set_partner (ModelNode *p) { _partner = p; }
};

template <typename T, bool AGGR>
class ACM_Container : public std::list<T*>, public ModelNode {
  template <typename T_PTR>
  struct LessPtr {
    bool operator()(T_PTR e1, T_PTR e2) const { return *e1 < * e2; }
  };
public:
  typedef std::set<T*, LessPtr<T*> > Set;
  typedef std::list<T*> Base;
  typedef T Type;
private:
  bool _sorted;
  Set _sorted_elems;
public:
  ACM_Container () : _sorted(false) {}

  string key () const {
    string result = "";
    for (typename Base::const_iterator i = Base::begin (); i != Base::end (); ++i)
      result += (*i)->key ();
    if (result == "") result = "-|";
    return result;
  }

  void clear () {
    Base::clear ();
    _sorted_elems.clear ();
    _sorted = false;
  }
  void insert (T *n) {
    Base::push_back (n);
    if (_sorted) _sorted_elems.insert(n);
    if (AGGR) n->set_parent (get_parent ());
  }
  void remove (T *n) {
    Base::remove (n);
    if (_sorted) _sorted_elems.erase (n);
  }
  Set &get_sorted () {
    if (!_sorted) {
      for (typename Base::iterator i = Base::begin (); i != Base::end (); ++i)
        _sorted_elems.insert (*i);
      _sorted = true;
    }
    return _sorted_elems;
  }
  void to_xml (SaveContext &context) {
    for (typename Base::iterator i = Base::begin (); i != Base::end (); ++i) {
      if (!(*i)->get_xml_node ()) {
        xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)(*i)->type_str (), NULL);
        (*i)->set_xml_node (node);
      }
      (*i)->to_xml (context);
      (*i)->set_xml_dirty (false);
    }
  }
};

#endif // __ModelNode_h__
