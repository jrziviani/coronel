#ifndef ILIST_HPP
#define ILIST_HPP

#include "memory/allocators.hpp"
#include "stdint.hpp"

namespace lib
{
    template <typename T>
    class ilist
    {
        struct node
        {
            T     data;
            node *next;

            node() :
                next(nullptr)
            {}

            node(const T &data) :
                data(data),
                next(nullptr)
            {}
        };

        class iterator
        {
            node *current_;

        public:
            iterator(node *current) :
                current_(current)
            {}

            iterator &operator++()
            {
                if (current_) {
                    current_ = current_->next;
                }
                return *this;
            }

            T &operator*() const
            {
                if (current_) {
                    return current_->data;
                }

                // TODO: handle error case, this is broken but it's not
                // in use yet.
                return *(T *) nullptr;
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

    public:
        using const_iterator = iterator;

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
            if (!tail_) {
                auto place = placement_kalloc(sizeof(node));
                tail_ = new (place) node(data);
                head_ = tail_;
            }
            else {
                auto place = placement_kalloc(sizeof(node));
                tail_->next = new (place) node(data);
                tail_       = tail_->next;
            }

            ++size_;
        }

        void push_front(const T &data)
        {
            (void) data;
        }

        void clear()
        {
            while (head_) {
                node *temp = head_;
                head_       = head_->next;
                delete temp;
            }

            head_ = tail_ = nullptr;
            size_ = 0;
        }

        const_iterator begin() const
        {
            return iterator(head_);
        }

        const_iterator end() const
        {
            return iterator(nullptr);
        }
    };
}

#endif // ILIST_HPP