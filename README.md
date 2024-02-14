# Extensions for Rofi

Extension for [Rofi](https://github.com/davatorium/rofi) written in C.

## Installation

```bash
# Install in default directory $HOME/.config/rofi
make install

# Install in directory ~/.rofi
make install INSTALLDIR=~/.rofi

# launch rofi with websearch extension
rofi -show drun -modi "drun,websearch:$PWD/websearch.o"

# launch rofi with websearch extension and google search engine
export SEARCH_FMT="https://www.google.com/search?q=test"
rofi -show drun -modi "drun,websearch:$PWD/websearch.o"
```

## Extensions

### Websearch

Search the web with you favourite search engine. Executes the command provided by environment variables with default values. You can also change them to your prefered settings.

```bash
# Command:  $BROWSER_CMD $BROWSER_ARGS $SEARCH_FMT
# Defaults: firefox      new-tab       https://duckduckgo.com/?q=%s
firefox new-tab https://duckduckgo.com/?q=%s
```

| Variable | Default | Description |
| ---  | --- | --- |
| BROWSER_CMD | firefox | The command to the browser executable. |
| BROWSER_ARGS | new-tab | The arguments to pass to the browser command. |
| SEARCH_FMT | https://duckduckgo.com/?q=%s | The url that is called with the format %s string that will be replaced by the search term later. |

## Development

```bash
# install gcc for compilation, valgrind for memcheck 
sudo pacman -S gcc valgrind

# execute tests
make test
```
