#ifndef LINKEDLIST_H_INCLUDED_
#define LINKEDLIST_H_INCLUDED_

#include <cstddef>

/******************************************************************************
 * @Description: Cau truc Node luu tru phan tu trong danh sach lien ket don
 * (Tuan thu C++ Coding Standard V2: tien to _ cho thanh vien lop)
 ******************************************************************************/
template <typename T>
struct Node {
    T _data;
    Node<T>* _pNext;

    /**********************************************************
     * @Description Constructor khoi tao Node voi gia tri du lieu
     * @param data Du lieu can luu vao Node
     **********************************************************/
    Node(const T& data) : _data(data), _pNext(nullptr) {}
};

/******************************************************************************
 * @Description: Lop Template Danh sach lien ket don Generic LinkedList<T>
 * Khong su dung STL container, quan ly bo nho dong chat che bang con tro _pTail
 * giup them cuoi O(1). Vo hieu hoa copy semantics de chong loi Double Free.
 * (Nhiem vu cot loi cua Thanh vien B - Tri)
 ******************************************************************************/
template <typename T>
class LinkedList {
private:
    Node<T>* _pHead;
    Node<T>* _pTail;
    int _iSize;

public:
    /**********************************************************
     * @Description Constructor mac dinh khoi tao danh sach rong
     **********************************************************/
    LinkedList() : _pHead(nullptr), _pTail(nullptr), _iSize(0) {}

    /**********************************************************
     * @Description Destructor tu dong thu hoi 100% vung nho cua cac node
     **********************************************************/
    ~LinkedList() {
        this->clear();
    }

    /**********************************************************
     * @Description Vo hieu hoa Copy Constructor de ngan ngua loi Double Free
     **********************************************************/
    LinkedList(const LinkedList<T>&) = delete;

    /**********************************************************
     * @Description Vo hieu hoa Copy Assignment de ngan ngua loi Double Free
     **********************************************************/
    LinkedList<T>& operator=(const LinkedList<T>&) = delete;

    /**********************************************************
     * @Description Kiem tra danh sach co dang rong khong
     * @return true neu danh sach khong co phan tu nao
     **********************************************************/
    bool isEmpty() const {
        return (this->_pHead == nullptr);
    }

    /**********************************************************
     * @Description Lay so luong phan tu hien tai trong danh sach
     * @return So luong phan tu kieu int
     **********************************************************/
    int getSize() const {
        return this->_iSize;
    }

    /**********************************************************
     * @Description Lay con tro node dau tien cua danh sach
     * @return Con tro Node<T>*
     **********************************************************/
    Node<T>* getHead() const {
        return this->_pHead;
    }

    /**********************************************************
     * @Description Lay con tro node cuoi cung cua danh sach
     * @return Con tro Node<T>*
     **********************************************************/
    Node<T>* getTail() const {
        return this->_pTail;
    }

    /**********************************************************
     * @Description Them mot phan tu vao cuoi danh sach voi do phuc tap O(1)
     * @param item Gia tri can them vao
     * @return void
     **********************************************************/
    void addTail(const T& item) {
        Node<T>* pNewNode = new Node<T>(item);
        if (this->isEmpty()) {
            this->_pHead = pNewNode;
            this->_pTail = pNewNode;
        } else {
            this->_pTail->_pNext = pNewNode;
            this->_pTail = pNewNode;
        }
        this->_iSize++;
    }

    /**********************************************************
     * @Description Tim kiem phan tu dau tien thoa man vi tu Predicate
     * @param pred Ham hoac Lambda bieu thuc kiem tra (nhan const T&, tra ve bool)
     * @return Con tro T* toi du lieu goc ben trong node, hoac nullptr neu khong tim thay
     **********************************************************/
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

    /**********************************************************
     * @Description Tim kiem phan tu dau tien (phien ban const)
     **********************************************************/
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

    /**********************************************************
     * @Description Xoa phan tu dau tien thoa man vi tu Predicate
     * va giai phong bo nho cua node do
     * @param pred Ham hoac Lambda bieu thuc kiem tra
     * @return true neu xoa thanh cong, false neu khong tim thay
     **********************************************************/
    template <typename Predicate>
    bool removeIf(Predicate pred) {
        if (this->isEmpty()) {
            return false;
        }

        Node<T>* pCurrent = this->_pHead;
        Node<T>* pPrev = nullptr;

        while (pCurrent != nullptr) {
            if (pred(pCurrent->_data)) {
                // Truong hop xoa node dau danh sach
                if (pPrev == nullptr) {
                    this->_pHead = pCurrent->_pNext;
                    if (this->_pHead == nullptr) {
                        this->_pTail = nullptr;
                    }
                } else {
                    pPrev->_pNext = pCurrent->_pNext;
                    // Truong hop xoa node cuoi danh sach
                    if (pCurrent == this->_pTail) {
                        this->_pTail = pPrev;
                    }
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

    /**********************************************************
     * @Description Giai phong toan bo cac node va dua danh sach ve trang thai rong
     * @return void
     **********************************************************/
    void clear() {
        Node<T>* pCurrent = this->_pHead;
        while (pCurrent != nullptr) {
            Node<T>* pNextNode = pCurrent->_pNext;
            delete pCurrent;
            pCurrent = pNextNode;
        }
        this->_pHead = nullptr;
        this->_pTail = nullptr;
        this->_iSize = 0;
    }
};

#endif // LINKEDLIST_H_INCLUDED_
