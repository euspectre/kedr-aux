#ifndef DO_TRACE_OPERATION_HH
#define DO_TRACE_OPERATION_HH

#include "trace.hh"
#include "trace_operation_include/trace_operation.hh"

#include <vector>

template <class TraceClass>
void doTraceOperation(TraceOperation& op,
    const std::vector<TraceClass>& operands, TraceClass& result);

// Итератор по сразу нескольким картам
// В качестве значения (iter->second) такой итератор содержит вектор указателей на значения из заданных карт,
// где индекс элемента в векторе соответствует индексу карты.
// При этом нулевой указатель означает, что в соответствующей карте данный ключ отсутствует.
template<class MapClass>
class MapVectorIterator
{
public:
    // Итератор рассчитан только на прямой проход.
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<typename MapClass::key_type, std::vector<const typename MapClass::mapped_type*>>;
    // Операция по вычитанию итераторов неприменима, поэтому тип difference_type бессмысленен. Но надо что-то записать сюда.
    using difference_type = std::ptrdiff_t;
    // Непонятно, зачем эти типы.
    using pointer = std::vector<typename MapClass::value_type*>*;
    using reference = std::vector<typename MapClass::value_type*>&;

    // Префиксная форма инкрементирования итератора.
    // Суффиксная форма не поддерживается.
    MapVectorIterator& operator++(void)
    {
        int n = iters.size();
        for (int i = 0; i < n; i ++)
        {
            if (val.second[i]) iters[i]++;
        }

        toFirstKey();

        return *this;
    }
    // Оператор разыменования.
    value_type* operator->(void) {return &val;}
    // Сравнение с другим итератором (равенство/неравенство)
    bool operator==(const MapVectorIterator& other)
    {
        if (firstValueIndex != other.firstValueIndex)
        {
            return false;
        }
        if (firstValueIndex != (int)iters.size())
        {
            return iters[firstValueIndex] == other.iters[firstValueIndex];
        }

        return true;
    }

    bool operator!=(const MapVectorIterator& other)
    {
        return !this->operator==(other);
    }

    // По заданному списку указателей на карты возвращает итератор, соответствующий первому элементу.
    // Любой из указателей на карты может быть NULL, это эквивалентно пустой карте.
    static MapVectorIterator begin(std::vector<const MapClass*> maps)
    {
        int n = maps.size();
        MapVectorIterator iter(typename MapClass::key_type(), n);
        for (int i = 0; i < n; i ++)
        {
            const MapClass* mapPointer = maps[i];
            if (mapPointer)
            {
                iter.iters[i] = mapPointer->begin();
                iter.itersEnd[i] = mapPointer->end();
            }
        }

        iter.toFirstKey();

        return iter;
    }

    // По заданному списку указателей на карты возвращает итератор, идущий после последнего элемента.
    // Любой из указателей на карты может быть NULL, это эквивалентно пустой карте.
    static MapVectorIterator end(std::vector<const MapClass*> maps)
    {
        int n = maps.size();
        MapVectorIterator iter(typename MapClass::key_type(), n);
        return iter;
    }
private:
    // Значение, которое возвращает операция разыменования итератора
    value_type val;
    // Итераторы по значениям отдельных карт, которые либо соответствуют текущему ключу
    // (соответствующий элемент в val.second отличен от NULL),
    // либо соответствуют элементу с минимальным ключом, который больше текущего.
    std::vector<typename MapClass::const_iterator> iters;
    // Итераторы, указывающие на конец отдельных карт.
    std::vector<typename MapClass::const_iterator> itersEnd;
    // Индекс первого итератора в массиве iters, который соответствует текущему ключу.
    // Значение, равное размеру массива iters, означает, что текущему ключу не соответствует ни один из итераторов.
    // Это верно для итератора end(), а также в качестве промежуточного состояния итератора перед вызовом toFirstKey().
    int firstValueIndex;

    // Конструирует итератор, который не указывает ни на один элемент.
    MapVectorIterator(typename MapClass::key_type key, size_t n): val(key, n), iters(n), itersEnd(n), firstValueIndex(n) {}
    // Учитывая состояние итераторов в iters, определяет минимальный ключ, который принадлежит хотя бы одной карте,
    // и заполняет поля val и firstValueIndex.
    // Текущее значение val и firstValueIndex игнорируется.
    void toFirstKey(void)
    {
        int n = iters.size();
        // Значение firstValueIndex, означающее, что все итераторы из iters указывают на конец соответствующих карт.
        firstValueIndex = n;
        for (int i = 0; i < n; i ++)
        {
            // Пропускаем
            if (iters[i] == itersEnd[i]) continue;
            
            typename MapClass::key_type key = iters[i]->first;

            if (firstValueIndex == n)
            { // Первый итератор, который указывает на значение.
                firstValueIndex = i;
                val.first = key;
                val.second[i] = &iters[i]->second;
            }
            else if (key < val.first)
            { // Итератор не первый, но его ключ меньше всех из уже обработанных
                firstValueIndex = i;
                val.first = key;
                val.second[i] = &iters[i]->second;
                // Очистку уже установленных указателей в val откладываем до конца цикла
            }
            else if (val.first < key)
            { // Ключ итератора больше, чем уже найденный. Игнорируем
              // NOTE: нельзя было опустить эту ветвь и использовать сравнение == в следующей ветви: для ключей карты
              // гарантирован только оператор '<'.
            }
            else
            { // нашли значение с ключом, который пока считается минимальным.
                val.second[i] = &iters[i]->second;
            }
        }
        // Очищаем все остальные указатели в val.
        for (int i = 0; i < firstValueIndex; i++)
        {
            val.second[i] = NULL;
        }
    }
};

#endif /* DO_TRACE_OPERATION_HH */
