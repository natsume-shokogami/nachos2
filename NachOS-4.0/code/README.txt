Cách chạy chương trình: Vào thư mục test, gõ ./nachos -x 'chương trình cần chạy' 
(./nachos trong test thực chất là 1 symlink tới nachos trong build.linux)
Lỗi: 
 - Thỉnh thoảng chương trình shell bị treo, có thể ấn Ctrl+C để thoát.

 - Chương trình cũng bị treo nếu AddrSpace cố execute một file không tồn tại 
(mặc dù em đã có thêm phần kiểm tra vào trong phương thức PTable::ExecUpdate, 
vì một lỗi nào đó tên file pass vào AddrSpace dùng để chạy chương trình bị sai).

 - System call ExecV vẫn chưa chạy được, tiến trình con vẫn không nhận được argc và argv
(cố gắng truy cập argc trong test_execv_child_thread tiến trình con sẽ thoát với exit code là 5, 
tức là IllegalAddressException, tức là tiến trình test_execv_child_thread chưa nhận được argc và argv được khởi tạo từ tiến trình cha)

Một số nguồn tham khảo:
https://github.com/47dev47null/nachos
https://github.com/CatLe2003/Nachos-System-calls-for-Multiprogramming
https://github.com/Toan2003/Nachos-project-2---OS-subject---HCMUS
Các tài liệu hệ điều hành và hướng dẫn đồ án.

Các system call:
Thay đổi việc cấp phát các page bộ nhớ, chuyển sang virtual memory:
 - Dùng gPhysPageBitMap để đánh dấu các frame bộ nhớ vật lý đã cấp phát.
 - Thay đổi quá trình nạp chương trình để từ vị trí virtual memory sang physical memory.

Thêm và thay đổi các exception khác để thay vì dừng NachOS chỉ dừng tiến trình hiện tại.
 - Nhận giá trị exception và exit code tương ứng, đồng thời tìm PID của tiến trình đang có exception.
 - Dùng phương thức PTable::ExitUpdate() (cũng là phương thức thực hiện Syscall Exit) để thoát tiến trình hiện tại 
và trả về exit code tương ứng với lỗi.
 - Các phương thức định thời các thread tiếp theo để chạy (ví dụ FindNextToRun trong synch.cc)
 sẽ đảm nhận các việc khôi phục các giá trị tiến trình cha, do đó không cần phải làm gì thêm.

Syscall Exec:
 - Nhận tên file từ người dùng và kiểm tra sự tồn tại của file, tìm vị trí trống trong PTable, khởi tạo PCB với tiến trình cha
là PID tiến trình đang chạy, và PID là vị trí trong PTable.
 - Khởi tạo một thread trong PCB mới tạo, dùng phương thức Fork() trong lớp Thread với hàm StartProcess_1
(hàm khởi tạo không gian bộ nhớ ảo để nạp và thực thi tiến trình).
 - Sử dụng semaphore bmsem để chỉ một tiến trình có thể thay đổi PTable cùng một lúc.
 - Trả về PID của tiến trình mới nếu thành công, -1 nếu thất bại.

Syscall Join:
 - Kiểm tra tính hợp lệ của PID muốn Join (Có trong PTable, không tự join chính nó,...)
 - Tăng numwait của tiến trình cha, dùng phương thức PCB::JoinWait để block tiến trình cha cho đến khi tiến trình con
hoàn thành chờ tiến trình con dùng phương thức PCB::JoinRelease.
 - Trả về exit code cho tiến trình cha, dùng phương thức ExitRelease để cho phép tiến trình con thoát.


Syscall Exit:
 - Nhận exit code từ userspace.
 - Kiểm tra tiến trình hiện tại, nếu tiến trình là tiến trình chính (tiến trình PID 0), thì dừng NachOS.
 - Nếu không, gọi PCB::SetExitCode đặt exit code cho tiến trình đang chạy,
  đồng thời gọi các phương thức PCB::JoinRelease để giải phóng tiến trình cha đang chờ và ExitWait để chờ tiến trình cha nhận
exit code tiến trình con.
 - Sau khi nhận exit code và tiến trình con thoát, dùng phương thức PCB::Remove để xóa PCB.

Syscall CreateSemaphore:
 - Kiểm tra tên semaphore, tìm semaphore trống, nếu có semaphore trùng tên hoặc không còn chỗ trống trả về -1.
 - Tạo semaphore với giá trị ban đầu, nếu thành công trả về vị trí semaphore trong STable, không trả về -1.

Syscall Wait, Signal:
 - Nhận tên semaphore từ userspace, kiểm tra semaphore có tên tương ứng có trong STable, nếu không trả về -1 báo lỗi.
 - Thực hiện các phương thức Sem::Wait hoặc Sem::Signal tương ứng tại vị trí semaphore có tên tương ứng.

Syscall ExecV:
 - Giống syscall Exec, có điều sẽ nhận argv và argc từ userspace, tập tin cần thực thi là argv[0]
 - Trong quá trình tạo không gian bộ nhớ ảo để thực thi tiến trình, đặt các giá trị argv vào cuối bộ nhớ 
 stack address space, đồng thời đặt register 4 và 5 lần lượt là argc và địa chỉ của argv trong bộ nhớ stack của tiến trình.





