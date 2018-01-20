/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONCATITERATOR_H
#define CONCATITERATOR_H

#include <list>

template <class T> class ConcatIterator {

private:
    typedef std::list<T> TList;
    typedef TList* TListPointer;
    typedef typename TList::iterator TList_Iter;
    typedef std::list< TListPointer> TListOfList;
    typedef typename TListOfList::iterator TListOfList_Iter;


    TList_Iter listIter;
    TListOfList_Iter listOfListIter;
    TListOfList listOfList;
public:
    ConcatIterator() = default;
    ~ConcatIterator() = default;

    void addList(std::list<T>& _List) {
        if(_List.empty()) {
            // ignore this list
            return;
        }

        if(listOfList.empty()) {
            listOfList.push_back(&_List);
            listOfListIter = listOfList.begin();
            listIter = (*listOfListIter)->begin();
        } else {
            listOfList.push_back(&_List);
        }
    }

    bool isIterationFinished() {
        if(listOfList.empty()) {
            return true;
        }

        if(listOfListIter == listOfList.end()) {
            return true;
        }

        return false;
    }

    T operator* () {
        if(isIterationFinished()) {
            THROW(std::out_of_range, "ConcatIterator::operator*(): Cannot dereference because iteration is finished.");
        }

        return *listIter;
    }

    void operator++ () {
        if(isIterationFinished()) {
            THROW(std::out_of_range, "ConcatIterator::operator++(): Cannot increment because iteration is finished.");
        }

        ++listIter;
        if(listIter == (*listOfListIter)->end()) {
            // use next list
            ++listOfListIter;
            if(listOfListIter == listOfList.end()) {
                return;
            }

            listIter = (*listOfListIter)->begin();
        }
    }
};

#endif // CONCATITERATOR_H
