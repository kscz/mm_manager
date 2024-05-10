#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct shadybank_client shadybank_client;

struct shadybank_client *shadybank_get_client(const char *shadybank_url);

int32_t shadybank_login(struct shadybank_client *client_handle, const char *account_id, const char *password);

int32_t shadybank_logout(struct shadybank_client *client_handle);

int32_t shadybank_credit(struct shadybank_client *client_handle, const char *magstripe, double amount);

char *shadybank_authorize_stripe(struct shadybank_client *client_handle,
                                 const char *magstripe,
                                 double amount);

char *shadybank_authorize_pan_shotp(struct shadybank_client *client_handle,
                                    const char *pan,
                                    const char *shotp,
                                    double amount);

int32_t shadybank_void(struct shadybank_client *client_handle, const char *auth_code);

int32_t shadybank_capture(struct shadybank_client *client_handle, double amount, const char *auth_code);

void shadybank_free_auth_code(char *auth_code);
