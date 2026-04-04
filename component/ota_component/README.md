# OTA_Component

> Inateck 自定义固件OTA

### 文件目录说明

- crc16：用于crc16包校验
- Inateck_ota：ota主体升级程序
- Inateck_ProductCatalog：产品目录表

### Git事项

#### 1. 添加子模块

添加子模块前，需要将子模块已存在的文件目录删除，后再运行如下命令添加子模块。

```bash
git submodule add https://gitlab.licheng-tech.com/hardware/firmware/tools/ota_component.git SDK/ota_component
```

添加子模块后提交内容即可

#### 2. 拉取时同步拉取子模块

要在使用 `git clone` 命令时同时拉取子模块（submodule），添加 `--recurse-submodules` 选项。这样会自动初始化和更新子模块。以下是具体的命令：

```bash
git clone --recurse-submodules <repository_URL>
```

请将 `<repository_URL>` 替换为包含子模块的存储库的 URL。通过添加 `--recurse-submodules` 选项，Git 将会在克隆存储库时自动初始化并更新子模块。

#### 3. 更新子模块

如果本地子模块目录为空，需要运行命令手动初始化

```bash
git submodule update --init --remote
```

拉取主项目时同时拉取子模块

```bash
git pull --recurse-submodule
```

#### 4. 删除子模块

- 本地目录下删除submodule的子目录：rm -rf submodule
- 编辑.gitmodules文件，将submodule相关的内容删除
- 删除.git/config文件里submodule相关的内容
- 删除.git/modules/submodule目录：rm -rf .git/modules/submodule
- 将变动内容提交即可。

```bash
git submodule deinit -f <submodule_path>
rm -rf .git/modules/<submodule_path>
git rm -f <submodule_path>
```

