#include <thread>
#include <future>
#include <iostream>
#include <vector>
#include <chrono>
#include <semaphore> // ��� ��������� ��������
using namespace std;

void work(vector<__int16>& arr, binary_semaphore& semaphore, condition_variable_any& cv, bool& work_ready) {
    semaphore.acquire(); // ���������� ��� ������ � �������

    // ��������� � ������������ ��������� �������� ��� ������
    int sleep_time;
    cout << "Enter the sleep time between elements (in milliseconds): ";
    cin >> sleep_time;

    // ����� ��������� ����� �� 12 � ������ ��������� �� 0
    int j = 0; // ������ ��� �������
    for (int i = 0; i < arr.size(); ++i) {
        if (arr[i] <= 12) {
            arr[j] = arr[i] * arr[i]; // ��������� ��������
            j++;
        }
    }

    // ��������� ������� ������� ������
    while (j < arr.size()) {
        arr[j] = 0;
        j++;
    }

    work_ready = true;
    cout << endl << "Work is done!" << endl;
    semaphore.release(); // ����������� �������
    cv.notify_all(); // ���������� ��������� ������
}

void countElements(vector<__int16>& arr, binary_semaphore& semaphore, condition_variable_any& cv, bool& work_ready, promise<int>&& prom) {
    semaphore.acquire();
    cv.wait(semaphore, [&work_ready] { return work_ready; }); // ����, ���� work �������� ���� ������

    // ������� ��������� ���������
    int non_zero_count = 0;
    for (int i = 0; i < arr.size(); ++i) {
        if (arr[i] != 0) {
            non_zero_count++;
        }
    }

    cout << endl << "Count elements is done!" << endl;
    prom.set_value(non_zero_count); // ���������� ��������� � main
    semaphore.release(); // ����������� �������
}

int main() {
    binary_semaphore semaphore(1); 
    condition_variable_any cv;
    bool work_ready = false;
    int n;
    promise<int> prom;
    future<int> fut = prom.get_future();

    // ������� ������
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

    // ��������� ������
    thread wThread(work, ref(arr), ref(semaphore), ref(cv), ref(work_ready));
    thread cThread(countElements, ref(arr), ref(semaphore), ref(cv), ref(work_ready), move(prom));

    // ���� ���������� ������ ������ work
    {
        semaphore.acquire();
        cv.wait(semaphore, [&work_ready] { return work_ready; });
        semaphore.release();
    }

    // ������� �������� ������
    cout << "New array after processing: " << endl;
    for (int i = 0; i < n; ++i) {
        cout << arr[i] << " ";
    }
    cout << endl;

    // �������� ��������� ������ ������ countElements
    int non_zero_count = fut.get();
    cout << "Number of non-zero elements: " << non_zero_count << endl;

    // ������� ���������� ���� �������
    wThread.join();
    cThread.join();

    return 0;
}
