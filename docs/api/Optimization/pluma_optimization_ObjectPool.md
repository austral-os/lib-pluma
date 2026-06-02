# Class `pluma::optimization::ObjectPool`

**Pre-allocates and recycles objects of type T.**

Uses a thread-safe freelist. Useful for high-frequency objects like

## Public Methods
- `ObjectPool()=default`
- `std::unique_ptr< T, void(*)(T *)> acquire(Args &&... args)` - *Acquires an object from the pool or allocates a new one if empty.*

