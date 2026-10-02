#ifndef LINKEDLIST_H_INCLUDED_
#define LINKEDLIST_H_INCLUDED_

/**
 * @Description: Cau truc Node cua danh sach lien ket don
 */
template <typename T>
struct Node {
    T _data;
    Node<T>* _pNext;

    Node(const T& data) : _data(data), _pNext(nullptr) {}
};

/**
 * @Description: Lop danh sach lien ket Generic Template tu quan ly bo nho
 */
template <typename T>
class LinkedList {
private:
    Node<T>* _pHead;
    Node<T>* _pTail;
    int _iSize;

public:
    /**
     * @Description: Khoi tao danh sach rong
     */
    LinkedList() : _pHead(nullptr), _pTail(nullptr), _iSize(0) {}

    /**
     * @Description: Huy danh sach va giai phong toan bo bo nho cac node
     */
    ~LinkedList() {
        this->clear();
    }

    /**
     * @Description: Xoa Copy Constructor de tranh Double Free
     */
    LinkedList(const LinkedList<T>&) = delete;

    /**
     * @Description: Xoa Copy Assignment Operator de tranh Double Free
     */
    LinkedList<T>& operator=(const LinkedList<T>&) = delete;

    /**
     * @Description: Ho tro Move Constructor
     */
    LinkedList(LinkedList<T>&& other) noexcept : _pHead(other._pHead), _pTail(other._pTail), _iSize(other._iSize) {
        other._pHead = nullptr;
        other._pTail = nullptr;
        other._iSize = 0;
    }

    /**
     * @Description: Ho tro Move Assignment Operator
     */
    LinkedList<T>& operator=(LinkedList<T>&& other) noexcept {
        if (this != &other) {
            this->clear();
            this->_pHead = other._pHead;
            this->_pTail = other._pTail;
            this->_iSize = other._iSize;

            other._pHead = nullptr;
            other._pTail = nullptr;
            other._iSize = 0;
        }
        return *this;
    }

    /**
     * @Description: Them phan tu vao cuoi danh sach voi do phuc tap O(1)
     * @param item: Gia tri can them
     */
    void addTail(const T& item) {
        Node<T>* pNewNode = new Node<T>(item);
        if (this->_pHead == nullptr) {
            this->_pHead = pNewNode;
            this->_pTail = pNewNode;
        } else {
            this->_pTail->_pNext = pNewNode;
            this->_pTail = pNewNode;
        }
        this->_iSize++;
    }

    /**
     * @Description: Xoa toan bo node va giai phong bo nho heap
     */
    void clear() {
        Node<T>* pCurrent = this->_pHead;
        while (pCurrent != nullptr) {
            Node<T>* pTemp = pCurrent;
            pCurrent = pCurrent->_pNext;
            delete pTemp;
        }
        this->_pHead = nullptr;
        this->_pTail = nullptr;
        this->_iSize = 0;
    }

    /**
     * @Description: Lay so luong phan tu hien tai O(1)
     * @return: So luong phan tu
     */
    int getSize() const {
        return this->_iSize;
    }

    /**
     * @Description: Kiem tra danh sach rong
     * @return: true neu rong, nguoc lai false
     */
    bool isEmpty() const {
        return this->_iSize == 0;
    }

    /**
     * @Description: Lay con tro toi node dau tien
     * @return: Con tro Node dau tien
     */
    Node<T>* getHead() const {
        return this->_pHead;
    }

    /**
     * @Description: Lay con tro toi node cuoi cung
     * @return: Con tro Node cuoi cung
     */
    Node<T>* getTail() const {
        return this->_pTail;
    }

    /**
     * @Description: Tim kiem phan tu theo dieu kien vi tu (Predicate)
     * @param pred: Ham hoac Lambda bieu thuc kiem tra dieu kien
     * @return: Con tro den phan tu tim thay, nullptr neu khong co
     */
    template <typename Predicate>
    T* findIf(Predicate pred) {
        Node<T>* pCurrent = this->_pHead;
        while (pCurrent != nullptr) {
            if (pred(pCurrent->_data)) {
                return &(pCurrent->_data);
            }
            pCurrent = pCurrent->_pNext;
        }
        return nullptr;
    }

    /**
     * @Description: Tim kiem phan tu hang (const) theo dieu kien vi tu
     * @param pred: Ham hoac Lambda bieu thuc kiem tra dieu kien
     * @return: Con tro hang den phan tu tim thay, nullptr neu khong co
     */
    template <typename Predicate>
    const T* findIf(Predicate pred) const {
        Node<T>* pCurrent = this->_pHead;
        while (pCurrent != nullptr) {
            if (pred(pCurrent->_data)) {
                return &(pCurrent->_data);
            }
            pCurrent = pCurrent->_pNext;
        }
        return nullptr;
    }

    /**
     * @Description: Xoa phan tu dau tien thoa man dieu kien vi tu
     * @param pred: Ham hoac Lambda bieu thuc kiem tra dieu kien
     * @return: true neu xoa thanh cong, nguoc lai false
     */
    template <typename Predicate>
    bool removeIf(Predicate pred) {
        Node<T>* pCurrent = this->_pHead;
        Node<T>* pPrev = nullptr;

        while (pCurrent != nullptr) {
            if (pred(pCurrent->_data)) {
                if (pPrev == nullptr) {
                    this->_pHead = pCurrent->_pNext;
                } else {
                    pPrev->_pNext = pCurrent->_pNext;
                }

                if (pCurrent == this->_pTail) {
                    this->_pTail = pPrev;
                }

                delete pCurrent;
                this->_iSize--;
                return true;
            }
            pPrev = pCurrent;
            pCurrent = pCurrent->_pNext;
        }
        return false;
    }

    /**
     * @Description: Lay phan tu tai vi tri chi muc iIndex
     * @param iIndex: Chi muc can lay (0-based)
     * @return: Con tro den phan tu, nullptr neu vuot qua pham vi
     */
    T* getAt(int iIndex) {
        if (iIndex < 0 || iIndex >= this->_iSize) {
            return nullptr;
        }
        Node<T>* pCurrent = this->_pHead;
        for (int i = 0; i < iIndex; i++) {
            pCurrent = pCurrent->_pNext;
        }
        return &(pCurrent->_data);
    }

    /**
     * @Description: Lay phan tu hang tai vi tri chi muc iIndex
     */
    const T* getAt(int iIndex) const {
        if (iIndex < 0 || iIndex >= this->_iSize) {
            return nullptr;
        }
        Node<T>* pCurrent = this->_pHead;
        for (int i = 0; i < iIndex; i++) {
            pCurrent = pCurrent->_pNext;
        }
        return &(pCurrent->_data);
    }
};

#endif // LINKEDLIST_H_INCLUDED_
