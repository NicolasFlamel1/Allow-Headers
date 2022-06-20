// Header files
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


// Structures

// Location configuration structure
typedef struct {

	// Enabled
	ngx_uint_t enabled;
	
	// Allowed headers
	ngx_array_t *allowedHeaders;
	
} ngx_http_allow_headers_conf_t;


// Function prototypes

// Allow header setup
static char *allowHeaderSetup(ngx_conf_t *configuration, ngx_command_t *command, void *data);

// Postconfiguration
static ngx_int_t postconfiguration(ngx_conf_t *configuration);

// Create location configuration
static void *createLocationConfiguration(ngx_conf_t *configuration);

// Merge location configuration
static char *mergeLocationConfiguration(ngx_conf_t *configuration, void *parent, void *child);

// Header filter
static ngx_int_t headerFilter(ngx_http_request_t *request);


// Constants

// Allow headers values
static ngx_conf_enum_t allowHeadersValues[] = {
	{
	
		// Off
		ngx_string("off"),
		0
	},
	{
	
		// On
		ngx_string("on"),
		1
	},
	{
	
		// End of value
		ngx_null_string,
		0
	}
};

// Directives
static ngx_command_t directives[] = {

	// Allow headers
	{
		// Name
		ngx_string("allow_headers"),
		
		// Type
		NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
		
		// Enumeration
		ngx_conf_set_enum_slot,
		
		// Configuration
		NGX_HTTP_LOC_CONF_OFFSET,
		
		// Offset
		offsetof(ngx_http_allow_headers_conf_t, enabled),
		
		// Values
		&allowHeadersValues
	},

	// Allow header
	{
	
		// Name
		ngx_string("allow_header"),
		
		// Type
		NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
		
		// Setup
		allowHeaderSetup,
		
		// Configuration
		NGX_HTTP_LOC_CONF_OFFSET,
		
		// Offset
		offsetof(ngx_http_allow_headers_conf_t, allowedHeaders),
		
		// Reserved
		NULL
	},
	
	// End of directives
	ngx_null_command
};

// Context
static ngx_http_module_t context = {

	// Preconfiguration
	NULL,
	
	// Postconfiguration
	postconfiguration,
	
	// Create main configuration
	NULL,
	
	// Initialize main configuration
	NULL,
	
	// Create server configuration
	NULL,
	
	// Merge server configuration
	NULL,
	
	// Create location configuration
	createLocationConfiguration,
	
	// Merge location configuration
	mergeLocationConfiguration
};

// Module
ngx_module_t ngx_http_allow_headers_module = {

	// Version header
	NGX_MODULE_V1,
	
	// Context
	&context,
	
	// Directives
	directives,
	
	// Type
	NGX_HTTP_MODULE,
	
	// initialize master
	NULL,
	
	// Initialize module
	NULL,
	
	// Initialize process
	NULL,
	
	// Initialize thread
	NULL,
	
	// Exit thread
	NULL,
	
	// Exit process
	NULL,
	
	// Exit master
	NULL,
	
	// Version footer
	NGX_MODULE_V1_PADDING
};


// Global variables

// Next header filter
static ngx_http_output_header_filter_pt nextHeaderFilter;


// Supporting function implementation

// Allow headers setup
char *allowHeaderSetup(ngx_conf_t *configuration, ngx_command_t *command, void *data) {

	// Get location configuration
	const ngx_http_allow_headers_conf_t *locationConfiguration = data;
	
	// Get arguments
	const ngx_str_t *arguments = configuration->args->elts;
	
	// Get header
	const ngx_str_t *header = &arguments[1];
	
	// Get allowed headers
	ngx_array_t **allowedHeaders = (ngx_array_t **)((char *)locationConfiguration + command->offset);
	
	// Check if allowed headers doesn't exist
	if(!*allowedHeaders) {
	
		// Check if creating allowed headers failed
		*allowedHeaders = ngx_array_create(configuration->pool, 1, sizeof(ngx_str_t));
		if(!*allowedHeaders) {
		
			// Return configuration error
			return NGX_CONF_ERROR;
		}
	}
	
	// Check if adding value to allowed headers failed
	ngx_str_t *allowedHeader = ngx_array_push(*allowedHeaders);
	if(!allowedHeader) {
	
		// Return configuration error
		return NGX_CONF_ERROR;
	}
	
	// Set allowed header
	*allowedHeader = *header;
	
	// Check if header is invalid
	if(!header->len) {
	
		// Log error
		ngx_conf_log_error(NGX_LOG_EMERG, configuration, 0, "invalid parameter \"%V\"", header);
		
		// Return configuration error
		return NGX_CONF_ERROR;
	}
	
	// Return configuration ok
	return NGX_CONF_OK;
}

// Postconfiguration
ngx_int_t postconfiguration(ngx_conf_t *configuration) {

	// Append filter to filter chain
	nextHeaderFilter = ngx_http_top_header_filter;
	ngx_http_top_header_filter = headerFilter;
	
	// Return ok
	return NGX_OK;
}

// Create location configuration
void *createLocationConfiguration(ngx_conf_t *configuration) {

	// Check if creating location configuration failed
	ngx_http_allow_headers_conf_t *locationConfiguration = ngx_pcalloc(configuration->pool, sizeof(ngx_http_allow_headers_conf_t));
	if(!locationConfiguration) {
	
		// Return null
		return NULL;
	}
	
	// Configure location configuration
	locationConfiguration->enabled = NGX_CONF_UNSET_UINT;
	
	// Return location configuration
	return locationConfiguration;
}

// Merge location configuration
char *mergeLocationConfiguration(ngx_conf_t *configuration, void *parent, void *child) {

	// Initialize current and previous location configuration
	ngx_http_allow_headers_conf_t *currentLocationConfiguration = child;
	const ngx_http_allow_headers_conf_t *previousLocationConfiguration = parent;
	
	// Merge location configuration values
	ngx_conf_merge_uint_value(currentLocationConfiguration->enabled, previousLocationConfiguration->enabled, 0);

	if(!currentLocationConfiguration->allowedHeaders) {
		currentLocationConfiguration->allowedHeaders = previousLocationConfiguration->allowedHeaders;
	}
	
	// Return configuration ok
	return NGX_CONF_OK;
}

// Header filter
ngx_int_t headerFilter(ngx_http_request_t *request) {

	// Get location configuration
	const ngx_http_allow_headers_conf_t *locationConfiguration = ngx_http_get_module_loc_conf(request, ngx_http_allow_headers_module);
	
	// Check if module isn't used
	if(!locationConfiguration->enabled) {
	
		// Return next header filter
		return nextHeaderFilter(request);
	}
	
	// Go through all headers
	const ngx_list_part_t *part = &request->headers_out.headers.part;
	ngx_table_elt_t *header = part->elts;
	for(ngx_uint_t i = 0;; ++i) {
	
		// Check if at the end of the part
		if(i >= part->nelts) {
		
			// Check if at last part
			if(!part->next) {
			
				// Break
				break;
			}
			
			// Go to next part
			part = part->next;
			header = part->elts;
			i = 0;
		}
		
		// Check if header is invaid
		if(!header[i].hash) {
		
			// Continue
			continue;
		}
		
		// Initialize blocked header
		ngx_uint_t blockedHeader = 1;
		
		// Check if allowed headers exist
		if(locationConfiguration->allowedHeaders) {
		
			// Go through all allowed headers
			const ngx_str_t *allowedHeaders = locationConfiguration->allowedHeaders->elts;
			for(ngx_uint_t j = 0; j < locationConfiguration->allowedHeaders->nelts; ++j) {
			
				// Get allowed headers
				const ngx_str_t *allowedHeader = &allowedHeaders[j];
				
				// Check if header is allowed
				if(allowedHeader->len == header[i].key.len && !ngx_strncasecmp(allowedHeader->data, header[i].key.data, header[i].key.len)) {
				
					// Clear blocked header
					blockedHeader = 0;
					
					// Break
					break;
				}
			}
		}
		
		// Check if header is blocked
		if(blockedHeader) {
		
			// Log info
			ngx_log_error(NGX_LOG_INFO, request->connection->log, 0, "blocked header %V", &header[i].key);
		
			// Invalidate header
			header[i].hash = 0;
		}
	}
	
	// Return next header filter
	return nextHeaderFilter(request);
}
