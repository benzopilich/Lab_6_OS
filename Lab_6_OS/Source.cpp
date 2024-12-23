#include <thread>
#include <future>
#include <iostream>
#include <vector>
#include <chrono>
#include <semaphore> // для бинарного семафора
using namespace std;

void work(vector<__int16>& arr, binary_semaphore& semaphore, condition_variable_any& cv, bool& work_ready) {
    semaphore.acquire(); // блокировка для работы с данными

    // Запросить у пользователя временной интервал для отдыха
    int sleep_time;
    cout << "Enter the sleep time between elements (in milliseconds): ";
    cin >> sleep_time;

    // Поиск квадратов чисел до 12 и замена остальных на 0
    int j = 0; // Индекс для вставки
    for (int i = 0; i < arr.size(); ++i) {
        if (arr[i] <= 12) {
            arr[j] = arr[i] * arr[i]; // сохраняем значение
            j++;
        }
    }

    // Заполняем остаток массива нулями
    while (j < arr.size()) {
        arr[j] = 0;
        j++;
    }

    work_ready = true;
    cout << endl << "Work is done!" << endl;
    semaphore.release(); // освобождаем семафор
    cv.notify_all(); // уведомляем остальные потоки
}

void countElements(vector<__int16>& arr, binary_semaphore& semaphore, condition_variable_any& cv, bool& work_ready, promise<int>&& prom) {
    semaphore.acquire();
    cv.wait(semaphore, [&work_ready] { return work_ready; }); // ждем, пока work завершит свою работу

    // Подсчет ненулевых элементов
    int non_zero_count = 0;
    for (int i = 0; i < arr.size(); ++i) {
        if (arr[i] != 0) {
            non_zero_count++;
        }
    }

    cout << endl << "Count elements is done!" << endl;
    prom.set_value(non_zero_count); // отправляем результат в main
    semaphore.release(); // освобождаем семафор
}

int main() {
    binary_semaphore semaphore(1); 
    condition_variable_any cv;
    bool work_ready = false;
    int n;
    promise<int> prom;
    future<int> fut = prom.get_future();

    // Создаем массив
    cout << "Enter array size: " << endl;
    cin >> n;
    vector<__int16> arr(n);
    cout << "Enter array elements: " << endl;
    for (int i = 0; i < n; ++i) {
        cin >> arr[i];
    }
    cout << "Your array: " << endl;
    for (int i = 0; i < n; ++i) cout << arr[i] << " ";
    cout << endl;

    // Запускаем потоки
    thread wThread(work, ref(arr), ref(semaphore), ref(cv), ref(work_ready));
    thread cThread(countElements, ref(arr), ref(semaphore), ref(cv), ref(work_ready), move(prom));

    // Ждем завершения работы потока work
    {
        semaphore.acquire();
        cv.wait(semaphore, [&work_ready] { return work_ready; });
        semaphore.release();
    }

    // Выводим итоговый массив
    cout << "New array after processing: " << endl;
    for (int i = 0; i < n; ++i) {
        cout << arr[i] << " ";
    }
    cout << endl;

    // Получаем результат работы потока countElements
    int non_zero_count = fut.get();
    cout << "Number of non-zero elements: " << non_zero_count << endl;

    // Ожидаем завершения всех потоков
    wThread.join();
    cThread.join();

    return 0;
}
