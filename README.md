# cairopp, C++17 binding cairo

**cairopp** is a C++17 binding for [cairo](https://cairographics.org/).

## Features

- **Ease of use**: everything is in a single header, just drop the header in your source tree.
- **Strict binding**: the API is exactly the same as cairo with just unavoidable exceptions.
- **Quality of life**: additional structs for points, rectangles, colors, etc.

Everything is in the `cairo` namespace.

## How to

### Naming

The binding sticks to the original cairo library. A function `cairo_foo_do_something` is generally transformed in a member function `do_something` of class `foo` in the `cairo` namespace: `cairo::foo::do_something`. The rare exceptions are member functions that would be the same as classes or enumerations. In this case, a `get_` is added at the beginning.

The enumeration members lose their prefix, but as they are in an `enum class`, an enumeration member `CAIRO_BAR_BAZ` is generally transformed into `cairo::bar::baz`. The only exception is the enumeration members `CAIRO_SOMETHING_DEFAULT` that are transformed into `cairo::something::preset` as `default` is a keyword in C++.

As you have noted, the naming convention follows roughly the standard library naming convention (except for most getters that have a `get_` prefix).

### Contructors

To follow the [binding guidelines](https://cairographics.org/manual/language-bindings.html), many constructors are replaced with static member functions. In particular, all surfaces are created with static member functions.

### Overloads

For each member function that have `x` and `y` *or* `width` and `height`, an overload is provided with a `vec2f` (for `double`s) or `vec2i` (for `int`s) parameter. For each member function that have `x` and `y` *and* `width` and `height`, an overload is provided with a `rectf` (for `double`s) or `recti` (for `int`s) parameter For each member function that have a `red`, `green`, `blue` and possibly `alpha`, an overload is provided with a `color` parameter.

When two parameters represents an array and the size of the array, a templated overload is provided that use `std::data` and `std::size` to get the associated data and size of the templated container. So it can be used with `std::array`, `std::vector` and even with `std::initializer_list`.

### Missing things

They are a number of missing things, it can be because there are callbacks (and I have to find a good way to handle them), or because it's not yet here (e.g. the many surfaces and devices), or because I don't want to support them. Anyway, pull requests are welcome to complete the binding.

Known missing classes:

- `user_scaled_font`
- `user_font_face`
- `surface_observer`
- `raster_source_pattern`
- `region`

Known missing member functions:

- `*::get_reference_count`
- `*::get_user_data`
- `*::set_user_data`
- `surface::write_to_png_stream`
- `surface::set_mime_data`
- `surface::get_mime_data`
- `surface::supports_mime_type`
- `image_surface::create_from_png_stream`

## License

This binding is released under the MIT license.
