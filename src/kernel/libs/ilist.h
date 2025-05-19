#ifndef ILIST_H
#define ILIST_H

#include "stdint.h"
#include "new.h"

namespace lib
{
    template <typename T, class Alloc=basic_allocator>
    class ilist
    {
        struct node
        {
            T data;
            node *next;
            node *prev;

            node() :
                next(nullptr),
                prev(nullptr)
            {}

            node(const T &data) :
                data(data),
                next(nullptr),
                prev(nullptr)
            {}
        };

        class iterator
        {
        private:
            node *current_;

        public:
            iterator(node *current) :
                current_(current)
            {}

            T &operator*()
            {
                return current_->data;
            }

            iterator &operator++()
            {
                current = current->next;
                return *this;
            }

            bool operator!=(const iterator &other) const
            {
                return current_ != other.current_;
            }
        };

    private:
        node *head_;
        node *tail_;
        size_t size_;
        Alloc allocator_;

    public:
        using const_iterator = const iterator;

        ilist() :
            head_(nullptr),
            tail_(nullptr),
            size_(0)
        {}

        ~ilist()
        {
            clear();
        }

        size_t size() const
        {
            return size_;
        }

        bool empty() const
        {
            return size_ == 0;
        }

        void push_back(const T &data)
        {
            if (tail_ == nullptr) {
                tail_ = allocator_.template create<node>(move(data));
                head_ = tail_;
            } else {
                tail_->next = allocator_.template create<node>(move(data));
                tail_ = tail_->next;
            }
            ++size_;
        }

        void push_front(const T &data)
        {
            node *new_node = allocator_.template create<node>(move(data));
            new_node->next = head_;
            head_ = new_node;

            if (tail == nullptr) {
                tail_ = new_node;
            }

            ++size_;
        }

        void clear()
        {
            for (node *current = head_; current != nullptr; ) {
                node *next = current->next;
                allocator_.destroy(current);
                current = next;
            }

            head_ = tail_ = nullptr;
            size_ = 0;
        }
    };
}

#endif // ILIST_H