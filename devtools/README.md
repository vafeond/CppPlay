# IDE Container
Extends the development container to support an IDE, being LunarVim.

1. Create the IDE Docker container: 
  - From the project root (`sudo` may be needed to use Docker):
  - Build the development Docker image, `make cbuild`  
  - Build the IDE Docker image, `make ibuild`
  - Create the IDE Docker container, `make icreate`

2. Run and configure LunarVim
  - Enter the IDE Docker container, `make iexec`
  - Install LunarVim, `LV_BRANCH='release-1.3/neovim-0.9' bash <(curl -s https://raw.githubusercontent.com/LunarVim/LunarVim/release-1.3/neovim-0.9/utils/installer/install.sh)`
    - Check for latest install command
  - Symbolic link LunarVim application, `ln -s /root/.local/bin/lvim /usr/bin/lvim`
  - Run LunarVim, `lvim README.md` (assumes README.md exists)
  - Open LunarVim config `<leader> ; c`
  - Replace contents with `devtools/lvim_config.lua`
    - If complains about `lsp_signature`:
      - Comment out `require "lsp_signature"....` line below plugin config
      - Exit and reenter LunarVim to allow LSP Signature plugin to load
      - Uncomment line, restart LunarVim
  - Install `clangd`, `:LspInstall clangd`
  - Install `codelldb`, `:Mason`, enter `DAP` menu, find `codelldb`, `i` for install

