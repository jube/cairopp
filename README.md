# cairopp, C++17 binding for cairo

**cairopp** is a C++17 binding for [cairo](https://cairographics.org/) (>= 1.16).

## Features

- **Ease of use**: everything is in a single header, just drop the header in your source tree.
- **Strict binding**: the API is exactly the same as cairo with just unavoidable exceptions.
- **Quality of life**: additional structs for points, rectangles, colors, etc.

Everything is in the `cairo` namespace.

## How to

### Naming

The binding sticks to the original cairo library. A function `cairo_foo_do_something` is generally transformed in a member function `do_something` of class `Foo` in the `cairo` namespace: `cairo::Foo::do_something`. The `get_` prefix are removed.

The enumeration members lose their prefix, but as they are in an `enum class`, an enumeration member `CAIRO_BAR_BAZ` is generally transformed into `cairo::Bar::Baz`.

### Contructors

To follow the [binding guidelines](https://cairographics.org/manual/language-bindings.html), many constructors are replaced with static member functions. In particular, all surfaces are created with static member functions.

### Overloads

For each member function that have `x` and `y` *or* `width` and `height`, an overload is provided with a `Vec2F` (for `double`s) or `Vec2I` (for `int`s) parameter. For each member function that have `x` and `y` *and* `width` and `height`, an overload is provided with a `RectF` (for `double`s) or `RectI` (for `int`s) parameter For each member function that have a `red`, `green`, `blue` and possibly `alpha`, an overload is provided with a `Color` parameter.

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

### Versioning

For now, the binding does not handle different versions of cairo. It may change in the future.

## License

This binding is released under the MIT license.
