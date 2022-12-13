# json-parser
c++ linear time json parser

## Objective

Create a tree structure representing a specific json string in linear time.

Each node (```json_value```) in the tree should expose the following :
1. The field name (if applicable)
2. The value
3. The type of node
4. Any children the node might have

## json types

A json value can be one of 7 types :
null, true, false, number, string, array, object.

true and false will be condensed into the boolean type.
```
enum class json_type
{
  null,
  boolean,
  number,
  string,
  array,
  object
}
```

However for the purposes of parsing json, only 4 will be used as null and the boolean values are just special cases for the string type.

## Algorithm

### Insight

When iterating over the input string, at any point only a single node will be modified. More specifically if a node is incomplete(still being modified), so is its parent node. Importantly a node can only be complete when all of its children are also complete.

Using a node references (containing only information about the start and stop indexes needed to construct a node) in a stack, the algorithm only needs to actively search for either the beginning of a child node or the end of the current node at the top of the stack. 
