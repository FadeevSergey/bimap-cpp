#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <stdexcept>
#include <iostream>

namespace bmp {
    class left_tag;
    class right_tag;

    template<class T, class Tag>
    class base_node {
    public:
        explicit base_node(const T& value)
                : value(value)
                , size(1) {
        }

        explicit base_node(T&& value)
                : value(std::move(value))
                , size(1) {
        }

        friend bool operator==(const base_node& a, const base_node& b) {
            return a.get_value() == b.get_value();
        }

        friend bool operator!=(const base_node& a, const base_node& b) {
            return !(a == b);
        }

        const T& get_value() const {
            return value;
        }

        [[nodiscard]] std::size_t get_size() const {
            return size;
        }

        void set_size(std::size_t new_size) {
            size = new_size;
        }

        base_node* parent = nullptr;
        base_node* left = nullptr;
        base_node* right = nullptr;
        base_node* next = nullptr;
        base_node* prev = nullptr;

    private:
        T value;
        std::size_t size;
    };

    template<class LeftType, class RightType>
    class double_node : public base_node<LeftType, left_tag>, public base_node<RightType, right_tag> {
    public:
        double_node(const LeftType& left_value, const RightType& right_value)
                : base_node<LeftType, left_tag>(left_value)
                , base_node<RightType, right_tag>(right_value) {
        }

        double_node(LeftType&& left_value, const RightType& right_value)
                : base_node<LeftType, left_tag>(std::move(left_value))
                , base_node<RightType, right_tag>(right_value) {
        }

        double_node(const LeftType& left_value, RightType&& right_value)
                : base_node<LeftType, left_tag>(left_value)
                , base_node<RightType, right_tag>(std::move(right_value)) {
        }

        double_node(LeftType&& left_value, RightType&& right_value)
                : base_node<LeftType, left_tag>(std::move(left_value))
                , base_node<RightType, right_tag>(std::move(right_value)) {
        }
    };

    template<typename T, typename Tag, typename Cmp = std::less <T>>
    class tree {
    public:
        explicit tree(Cmp comparator = Cmp())
                : root(nullptr)
                , comparator(comparator) {
        }

        explicit tree(base_node<T, Tag>* root, Cmp comparator = Cmp())
                : root(root)
                , comparator(comparator) {
        }

        base_node<T, Tag>* get_first_node() const {
            base_node<T, Tag>* cur = root;
            while (cur != nullptr && cur->left != nullptr) {
                cur = cur->left;
            }
            return cur;
        }

        base_node<T, Tag>* get_last_node() const {
            base_node<T, Tag>* cur = root;
            while (cur != nullptr && cur->right != nullptr) {
                cur = cur->right;
            }
            return cur;
        }

        base_node<T, Tag>* get_root() const {
            return root;
        }

        void set_comparator(Cmp cmp) {
            comparator = cmp;
        }

        void insert(base_node<T, Tag>* value_node) {
            if (root == nullptr) {
                root = value_node;
                return;
            }

            base_node<T, Tag>* find_result = find_place(value_node->get_value());

            if (!comparator(find_result->get_value(), value_node->get_value()) &&
                !comparator(value_node->get_value(), find_result->get_value())) {
                return;
            }

            if (comparator(find_result->get_value(), value_node->get_value())) {
                find_result->right = value_node;
                find_result->right->parent = find_result;

                if (find_result->next != nullptr) {
                    find_result->next->prev = find_result->right;
                }
                find_result->right->next = find_result->next;

                find_result->next = find_result->right;
                find_result->right->prev = find_result;

                balance(find_result->right);
            } else {
                find_result->left = value_node;
                find_result->left->parent = find_result;

                if (find_result->prev != nullptr) {
                    find_result->prev->next = find_result->left;
                }
                find_result->left->prev = find_result->prev;

                find_result->prev = find_result->left;
                find_result->left->next = find_result;

                balance(find_result->left);
            }
        }

        void erase(const T& value) {
            base_node<T, Tag>* find_result = find_place(value);

            if (find_result == nullptr ||
                comparator(find_result->get_value(), value) ||
                comparator(value, find_result->get_value())) {
                return;
            }

            if (find_result->prev != nullptr) {
                find_result->prev->next = find_result->next;
            }

            if (find_result->next != nullptr) {
                find_result->next->prev = find_result->prev;
            }

            base_node<T, Tag>* R = nullptr;
            if (find_result != nullptr &&
                !comparator(find_result->get_value(), value) &&
                !comparator(value, find_result->get_value())) {
                balance(find_result);
                std::swap(R, root->right);
                if (R != nullptr) {
                    R->parent = nullptr;
                }
                fix_size(root);
            }

            root = root->left;

            if (root != nullptr) {
                root->parent = nullptr;
            }

            if (root == nullptr && R == nullptr) {
                return;
            }

            balance(get_last_node());

            if (root != nullptr) {
                root->right = R;
                if (root->right != nullptr) {
                    root->right->parent = root;
                }
            } else {
                root = R;
                root->parent = nullptr;
            }
            fix_size(root);
        }

        base_node<T, Tag>* find(const T& value) const {
            base_node<T, Tag>* find_result = find_place(value);
            return (find_result != nullptr &&
                    !comparator(find_result->get_value(), value) &&
                    !comparator(value, find_result->get_value())) ? find_result : nullptr;
        }

        base_node<T, Tag>* find_place(const T& value) const {
            if (root == nullptr) return nullptr;

            base_node<T, Tag>* cur = root;
            while (true) {
                if (comparator(cur->get_value(), value)) {
                    if (cur->right == nullptr) {
                        break;
                    } else {
                        cur = cur->right;
                    }
                } else if (comparator(value, cur->get_value())) {
                    if (cur->left == nullptr) {
                        break;
                    } else {
                        cur = cur->left;
                    }
                } else {
                    break;
                }
            }

            return cur;
        }

        [[nodiscard]] std::size_t size() const {
            return size(root);
        }

        friend void swap(tree<T, Tag, Cmp>& first, tree<T, Tag, Cmp>& second) {
            std::swap(first.root, second.root);
            std::swap(first.comparator, second.comparator);
        }

    private:
        [[nodiscard]] std::size_t size(const base_node<T, Tag>* cur) const {
            return (cur != nullptr) ? cur->get_size() : 0;
        }

        void fix_size(base_node<T, Tag>* cur) {
            if (cur != nullptr) {
                cur->set_size(1 + size(cur->left) + size(cur->right));
            }
        }

        void rotate(base_node<T, Tag>* cur) {
            if (cur == nullptr) {
                return;
            }
            if (cur->parent == nullptr) {
                fix_size(cur);
                return;
            }
            if (cur->parent->parent != nullptr) {
                if (cur->parent->parent->left == cur->parent) {
                    cur->parent->parent->left = cur;
                } else {
                    cur->parent->parent->right = cur;
                }
            }

            if (cur->parent->left == cur) {
                base_node<T, Tag>* P = cur->parent;
                base_node<T, Tag>* R = cur->right;
                cur->parent = P->parent;
                cur->right = P;
                P->parent = cur;
                P->left = R;
                if (R != nullptr) {
                    R->parent = P;
                }
            } else {
                base_node<T, Tag>* P = cur->parent;
                base_node<T, Tag>* L = cur->left;
                cur->parent = P->parent;
                cur->left = P;
                P->parent = cur;
                P->right = L;
                if (L != nullptr) {
                    L->parent = P;
                }
            }

            if (cur->parent == nullptr) {
                root = cur;
            }

            fix_size(cur->left);
            fix_size(cur->right);
            fix_size(cur);
        }

        void balance(base_node<T, Tag>* cur) {
            if (cur == nullptr) {
                return;
            }
            if (cur->parent == nullptr) {
                fix_size(cur);
                root = cur;
            } else if (cur->parent->parent == nullptr) {
                rotate(cur);
                root = cur;
            } else {
                rotate(cur->parent);
                rotate(cur);
                balance(cur);
            }
        }

        base_node<T, Tag>* root;
        Cmp comparator;
    };

    template<typename Left,
            typename Right,
            typename CompareLeft = std::less<Left>,
            typename CompareRight = std::less<Right>>
    class bimap {
    public:
        using left_t = Left;
        using right_t = Right;

        using left_node_t = base_node<Left, left_tag>;
        using right_node_t = base_node<Right, right_tag>;

        using left_tree_t = tree<left_t, left_tag, CompareLeft>;
        using right_tree_t = tree<right_t, right_tag, CompareRight>;

        using double_node_t = double_node<left_t, right_t>;

        class right_iterator;
        class left_iterator;

        class left_iterator {
        public:
            left_iterator(const bimap* bimap_ptr, left_node_t * node)
                    : node(node)
                    , bimap_ptr(bimap_ptr) {
            }

            left_iterator(const left_iterator& iterator)
                    : node(iterator.node)
                    , bimap_ptr(iterator.bimap_ptr) {
            }

            const left_t& operator*() const {
                return node->get_value();
            }

            left_iterator& operator++() {
                node = node->next;
                return *this;
            }

            left_iterator operator++(int) {
                auto result = *this;
                ++(*this);
                return result;
            }

            left_iterator& operator--() {
                node = (node == nullptr) ? bimap_ptr->left_tree.get_last_node() : node->prev;
                return *this;
            }

            left_iterator operator--(int) {
                auto result = *this;
                --(*this);
                return result;
            }

            friend bool operator==(const left_iterator& a, const left_iterator& b) noexcept {
                return a.node == b.node;
            }

            friend bool operator!=(const left_iterator& a, const left_iterator& b) noexcept {
                return !(a == b);
            }

            right_iterator flip() const {
                if (node == nullptr) {
                    return {bimap_ptr, nullptr};
                } else {
                    return {bimap_ptr, static_cast<double_node_t*>(node)};
                }
            }

            left_node_t* get_node() {
                return node;
            }

        private:
            friend class bimap;

            left_node_t* node;
            const bimap* bimap_ptr;
        };

        class right_iterator {
        public:
            right_iterator(const bimap* bimap_ptr, right_node_t* node)
                    : node(node)
                    , bimap_ptr(bimap_ptr) {
            }

            right_iterator(const right_iterator& iterator)
                    : node(iterator.node)
                    , bimap_ptr(iterator.bimap_ptr) {
            }

            const right_t& operator*() const {
                return node->get_value();
            }

            right_iterator& operator++() {
                this->node = node->next;
                return *this;
            }

            right_iterator operator++(int) {
                auto result = *this;
                ++(*this);
                return result;
            }

            right_iterator& operator--() {
                node = (node == nullptr) ? bimap_ptr->right_tree.get_last_node() : node->prev;
                return *this;
            }

            right_iterator operator--(int) {
                auto result = *this;
                --(*this);
                return result;
            }

            friend bool operator==(const right_iterator& a, const right_iterator& b) noexcept {
                return a.node == b.node;
            }

            friend bool operator!=(const right_iterator& a, const right_iterator& b) noexcept {
                return !(a == b);
            }

            left_iterator flip() const {
                if (node == nullptr) {
                    return {bimap_ptr, nullptr};
                } else {
                    return {bimap_ptr, static_cast<double_node_t*>(node)};
                }
            }

            right_node_t* get_node() {
                return node;
            }

        private:
            friend class bimap;

            right_node_t* node;
            const bimap* bimap_ptr;
        };

        explicit bimap(CompareLeft compare_left = CompareLeft(), CompareRight compare_right = CompareRight())
                : left_cmp(compare_left)
                , right_cmp(compare_right) {
            left_tree.set_comparator(compare_left);
            right_tree.set_comparator(compare_right);
        }

        bimap(const bimap& other)
                : left_cmp(other.left_cmp)
                , right_cmp(other.right_cmp) {
            left_tree.set_comparator(other.left_cmp);
            right_tree.set_comparator(other.right_cmp);

            left_iterator left_tree_iterator = other.begin_left();

            while (left_tree_iterator != other.end_left()) {
                this->insert(*left_tree_iterator, *left_tree_iterator.flip());
                ++left_tree_iterator;
            }
        }

        bimap(bimap&& other) noexcept {
            std::swap(bimap_size, other.bimap_size);

            swap(left_tree, other.left_tree);
            swap(right_tree, other.right_tree);

            std::swap(left_cmp, other.left_cmp);
            std::swap(right_cmp, other.right_cmp);
        }

        bimap& operator=(const bimap& other) {
            *this = bimap(other);

            return *this;
        }

        bimap& operator=(bimap&& other) noexcept {
            std::swap(bimap_size, other.bimap_size);

            swap(left_tree, other.left_tree);
            swap(right_tree, other.right_tree);

            std::swap(left_cmp, other.left_cmp);
            std::swap(right_cmp, other.right_cmp);

            return *this;
        }

        ~bimap() {
            subtree_deleter(left_tree.get_root());
        }

        left_iterator insert(const left_t& left, const right_t& right) {
            return basic_insert<const left_t, const right_t>(std::forward<const left_t>(left), std::forward<const right_t>(right));
        }

        left_iterator insert(const left_t& left, right_t&& right) {
            return basic_insert<const left_t, right_t>(std::forward<const left_t>(left), std::forward<right_t>(right));
        }

        left_iterator insert(left_t&& left, const right_t& right) {
            return basic_insert<left_t, const right_t>(std::forward<left_t>(left), std::forward<const right_t>(right));
        }

        left_iterator insert(left_t&& left, right_t&& right) {
            return basic_insert<left_t, right_t>(std::forward<left_t>(left), std::forward<right_t>(right));
        }

        left_iterator erase_left(left_iterator it) {
            auto next_iterator = ++(left_iterator(it));
            left_tree.erase(*it);
            right_tree.erase(*it.flip());
            --bimap_size;

            delete static_cast<double_node_t*>(it.get_node());

            return next_iterator;
        }

        bool erase_left(const left_t& left) {
            if (left_tree.find(left) == nullptr) {
                return false;
            }
            auto* node_ptr = left_tree.find(left);
            right_tree.erase(at_left(left));
            left_tree.erase(left);
            --bimap_size;

            delete static_cast<double_node_t*>(node_ptr);

            return true;
        }

        right_iterator erase_right(right_iterator it) {
            auto next_iterator = ++(right_iterator(it));
            right_tree.erase(*it);
            left_tree.erase(*it.flip());
            --bimap_size;

            delete static_cast<double_node_t*>(it.get_node());

            return next_iterator;
        }

        bool erase_right(const right_t& right) {
            if (right_tree.find(right) == nullptr) {
                return false;
            }
            auto* node_ptr = right_tree.find(right);
            left_tree.erase(at_right(right));
            right_tree.erase(right);
            --bimap_size;

            delete static_cast<double_node_t*>(node_ptr);

            return true;
        }

        left_iterator erase_left(left_iterator first, left_iterator last) {
            while (first != last) {
                first = erase_left(first);
            }

            return last;
        }

        right_iterator erase_right(right_iterator first, right_iterator last) {
            while (first != last) {
                first = erase_right(first);
            }

            return last;
        }

        left_iterator find_left(const left_t& left) const {
            left_node_t* node_ptr = left_tree.find(left);

            return left_iterator(this, node_ptr);
        }

        right_iterator find_right(const right_t& right) const {
            right_node_t* node_ptr = right_tree.find(right);

            return right_iterator(this, node_ptr);
        }

        const right_t& at_left(const left_t& key) const {
            if (left_tree.find(key) == nullptr) {
                throw std::out_of_range("Bimap does not contains left key");
            }

            return static_cast<right_node_t*>(static_cast<double_node_t*>(left_tree.find(key)))->get_value();
        }

        const left_t& at_right(const right_t& key) const {
            if (right_tree.find(key) == nullptr) {
                throw std::out_of_range("Bimap does not contains right key");
            }

            return static_cast<left_node_t*>(static_cast<double_node_t*>(right_tree.find(key)))->get_value();
        }

        template<typename T,
                typename = std::enable_if_t<std::is_same_v<T, right_t> && std::is_default_constructible_v<left_t>>>
        const right_t& at_left_or_default(const T& key) {
            if (left_tree.find(key) != nullptr) {
                return *left_iterator(this, left_tree.find_place(key)).flip();
            } else {
                right_t default_right = right_t();
                if (find_right(default_right) != end_right()) {
                    erase_right(default_right);
                }
                return *insert(key, default_right).flip();
            }
        }

        template<typename T,
                typename = std::enable_if_t<std::is_same_v<T, left_t> && std::is_default_constructible_v<right_t>>>
        const left_t& at_right_or_default(const T& key) {
            if (right_tree.find(key) != nullptr) {
                return *right_iterator(this, right_tree.find_place(key)).flip();
            } else {
                left_t default_left = left_t();
                if (find_left(default_left) != end_left()) {
                    erase_left(default_left);
                }

                return *insert(default_left, key);
            }
        }

        // не меньше
        left_iterator lower_bound_left(const left_t& left) const {
            left_node_t* find_res = left_tree.find_place(left);
            if (!left_cmp(find_res->get_value(), left)) {
                return left_iterator(this, find_res);
            } else {
                if (find_res->next == nullptr) {
                    return end_left();
                } else {
                    return left_iterator(this, find_res->next);
                }
            }
        }

        // больше
        left_iterator upper_bound_left(const left_t& left) const {
            left_node_t* find_res = left_tree.find_place(left);
            if (left_cmp(left, find_res->get_value())) {
                return left_iterator(this, find_res);
            } else if (!left_cmp(left, find_res->get_value()) && !left_cmp(find_res->get_value(), left)) {
                if (find_res->next == nullptr) {
                    return end_left();
                } else {
                    return left_iterator(this, find_res);
                }
            } else {
                if (find_res->next == nullptr) {
                    return end_left();
                } else {
                    return left_iterator(this, find_res->next);
                }
            }
        }

        // не меньше
        right_iterator lower_bound_right(const right_t& right) const {
            right_node_t* find_res = right_tree.find_place(right);
            if (!right_cmp(find_res->get_value(), right)) {
                return right_iterator(this, find_res);
            } else {
                if (find_res->next == nullptr) {
                    return end_right();
                } else {
                    return right_iterator(this, find_res->next);
                }
            }
        }

        // больше
        right_iterator upper_bound_right(const right_t& right) const {
            right_node_t* find_res = right_tree.find_place(right);
            if (right_cmp(right, find_res->get_value())) {
                return right_iterator(this, find_res);
            } else if (!right_cmp(right, find_res->get_value()) && !right_cmp(find_res->get_value(), right)) {
                if (find_res->next == nullptr) {
                    return end_right();
                } else {
                    return right_iterator(this, find_res);
                }
            } else {
                if (find_res->next == nullptr) {
                    return end_right();
                } else {
                    return right_iterator(this, find_res->next);
                }
            }
        }

        left_iterator begin_left() const {
            return left_iterator(this, left_tree.get_first_node());
        }

        left_iterator end_left() const {
            return left_iterator(this, nullptr);
        }

        right_iterator begin_right() const {
            return right_iterator(this, right_tree.get_first_node());
        }

        right_iterator end_right() const {
            return right_iterator(this, nullptr);
        }

        [[nodiscard]] std::size_t size() const {
            return bimap_size;
        }

        [[nodiscard]] bool empty() const {
            return size() == 0;
        }

        friend bool operator==(const bimap& a, const bimap& b) {
            if (a.size() != b.size()) {
                return false;
            }

            auto first_left_it = a.begin_left();
            auto second_left_it = b.begin_left();

            while (first_left_it != a.end_left() &&
                   second_left_it != b.end_left()) {
                if (a.left_cmp(*first_left_it, *second_left_it) ||
                    a.left_cmp(*second_left_it, *first_left_it) ||
                    b.right_cmp(*first_left_it.flip(), *second_left_it.flip()) ||
                    b.right_cmp(*second_left_it.flip(), *first_left_it.flip())) {
                    return false;
                }
                ++first_left_it;
                ++second_left_it;
            }

            return true;
        }

        friend bool operator!=(const bimap& a, const bimap& b) {
            return !(a == b);
        }

    private:
        void subtree_deleter(left_node_t* cur_root) {
            if (cur_root != nullptr) {
                subtree_deleter(cur_root->left);
                subtree_deleter(cur_root->right);

                delete static_cast<double_node_t*>(cur_root);
            }
        }

        template <class L, class R>
        left_iterator basic_insert(L&& left, R&& right) {
            if (left_tree.find(left) != nullptr || right_tree.find(right) != nullptr) {
                return end_left();
            } else {
                auto* new_double_node = new double_node_t(std::forward<L>(left), std::forward<R>(right));
                left_tree.insert(new_double_node);
                right_tree.insert(new_double_node);
                ++bimap_size;

                return {this, new_double_node};
            }
        }

        left_tree_t left_tree;
        right_tree_t right_tree;

        CompareLeft left_cmp;
        CompareRight right_cmp;

        std::size_t bimap_size = 0;
    };
}