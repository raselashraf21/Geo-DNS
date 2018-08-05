
#ifndef GEODNS_FACTORY_H
#define GEODNS_FACTORY_H

// Should be implemented by 'factory'
void make_app(void **app, char *config_path);
void free_app(void **app);

#endif // GEODNS_FACTORY_H
