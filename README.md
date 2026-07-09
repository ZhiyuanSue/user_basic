# user_basic

This is the user basic test get from 

https://github.com/Azure-stars/testsuits-for-oskernel/releases

and this repo is include the basic test only

Thanks to Azure

I have changed some thing for my test

## RendezvOS (initramfs / cpio mode)

See **[RENDEZVOS.md](RENDEZVOS.md)** for how ELFs are packed into `rootfs/tests/`
instead of kernel `link_app.o`. Execve tests use paths like `/tests/test_echo`.

The following is the original readme

## riscv syscalls测试用例

### 配置
先把riscv的交叉编译链的路径放到环境变量PATH中，例如`export PATH=$PATH:/path/to/kendryte-toolchain/bin`
<br>

### 编译
编译所有的syscalls测试用例：
```
cd user
./build-oscomp.sh
```
之后也可以使用脚本`src/oscomp/build-single-testcase.sh`来编译单个测例

### 运行
syscalls测试用例程序会生成到目录：
```
user/build/riscv64
```
把此文件夹放到待测OS的EXT4文件系统中即可；

## 🧪 测试用例验证准则（重要）

**所有用户态测试用例必须先在qemu-user上验证通过后，才能在RendezvOS内核上测试。**

### 为什么需要这个准则？

这个准则确保我们可以区分：
- **测试用例本身的bug** vs **RendezvOS内核实现的问题**
- 避免浪费时间调试实际上是测试用例错误的问题
- 确保测试用例在标准Linux环境下的行为是正确的

### 验证流程

#### 第一步：在qemu-user上验证
```bash
# 1. 编译静态链接版本
cd user_payload/user/build
make aarch64/test_signal_static  # 或其他架构

# 2. 在qemu-user上运行测试
qemu-aarch64-static ./aarch64/test_signal
# 或
qemu-x86_64-static ./x86_64/test_signal

# 3. 只有在qemu-user上通过后，才能在内核上测试
```

#### 第二步：在RendezvOS内核上测试
```bash
# 只有通过qemu-user验证的测试才能在这里运行
make ARCH=aarch64 run TEST=test_signal
```

### 支持的架构qemu-user命令

- **aarch64**: `qemu-aarch64-static` 或 `qemu-aarch64`
- **x86_64**: `qemu-x86_64-static` 或 `qemu-x86_64`
- **riscv64**: `qemu-riscv64-static` 或 `qemu-riscv64`

### 自动化验证脚本

使用提供的验证脚本自动测试所有用例：
```bash
cd user_payload
./scripts/verify_tests_on_qemu_user.sh aarch64
```

### 常见问题排查

如果测试用例在qemu-user上失败：
1. **不要在内核上调试** - 这是测试用例的问题
2. 先修复测试用例本身
3. 确保测试逻辑在标准Linux下正确
4. 只有在qemu-user通过后才能在内核上测试

如果测试用例在qemu-user上通过，但在内核上失败：
1. 这是RendezvOS内核实现的问题
2. 可以安全地在内核层面调试
3. 检查相关的系统调用实现
