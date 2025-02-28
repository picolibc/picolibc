int main(void) {
#if __has_builtin(__builtin_object_size)
return 0;
#else
return 1;
#endif
}
