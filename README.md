# cairopp, C++17 binding cairo

**cairopp** is a C++17 binding for [cairo](https://cairographics.org/).

- Ease of use: everything is in a single header, just drop the header in your source tree.
- Strict binding: the API is exactly the same as cairo with just unavoidable exceptions.
- Quality of life: additional structs for points, rectangles, etc.

Everything is in the `cairo` namespace.




`default` is generally transformed into `preset`


missing classes:

- `user_scaled_font`
- `user_font_face`
- `observer_surface`
- `raster_source_pattern`

missing methods:

- `surface::write_to_png_stream`
- `surface::set_mime_data`
- `surface::get_mime_data`
- `surface::supports_mime_type`

## License

This binding uses the MIT license.
