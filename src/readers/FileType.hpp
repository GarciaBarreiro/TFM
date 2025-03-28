// TODO: move to another place

// could probably add here the chooseReader/WriterType function
// on a single FileFactory with two classes defined

#pragma once

enum File_t
{
    txt_t,  // txt type
    las_t,  // las type
    err_t   // error type (no compatible extensions were found)
};