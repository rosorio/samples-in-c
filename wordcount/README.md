
## Syntax

`wordcount DICTFILE [FILE....]`
* DICTDILE : dictionary file
* FILE : file(s) to parse

## Description

The wordcount utility counts the number of occurrence of any word in the DICTFILE that
appears in the input streams, and also count the total number of words. Matching is performed on a full word basis. Words are delimited by white space and the search is case sensitive.

## How to build

```
# make all
```

## test

```
# make test
```
