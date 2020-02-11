
#include <stdlib.h>
#include <stdio.h>
#include "fsl_sss_api.h"


#define B64_IS_EQUAL   (uint8_t)64
#define B64_IS_INVALID (uint8_t)0xFF

/**
 * \brief Returns the base 64 character of the given index.
 * \param[in] id     index to check
 * \param[in] rules  base64 ruleset to use
 * \return the base 64 character of the given index
 */
char base64Char(uint8_t id, const uint8_t * rules)
{
    if (id < 26)
    {
        return (char)('A' + id);
    }
    if ((id >= 26) && (id < 52))
    {
        return (char)('a' + id - 26);
    }
    if ((id >= 52) && (id < 62))
    {
        return (char)('0' + id - 52);
    }
    if (id == 62)
    {
        return rules[0];
    }
    if (id == 63)
    {
        return rules[1];
    }

    if (id == B64_IS_EQUAL)
    {
        return rules[2];
    }
    return B64_IS_INVALID;
}


/** \brief Encode data as base64 string with ruleset option. */
sss_status_t base64encode(
    const uint8_t*  data,         /**< [in] The input byte array that will be converted to base 64 encoded characters */
    size_t          data_size,    /**< [in] The length of the byte array */
    char*           encoded,      /**< [in] The output converted to base 64 encoded characters. */
    size_t*         encoded_size /**< [inout] Input: The size of the encoded buffer, Output: The length of the encoded base 64 character string */
    )
{
    sss_status_t status = kStatus_SSS_Success;
    size_t data_idx = 0;
    size_t b64_idx = 0;
    size_t offset = 0;
    uint8_t id = 0;
    size_t b64_len;
    uint8_t rules[4]   = { '+', '/', '=', 64 };


    do
    {
        // Check the input parameters
        if (encoded == NULL || data == NULL || encoded_size == NULL )
        {
            status = kStatus_SSS_InvalidArgument;
            printf( "Null input parameter:%d", status);
        }

        // Calculate output length for buffer size check
        b64_len = (data_size / 3 + (data_size % 3 != 0)) * 4; // ceil(size/3)*4
        if (rules[3])
        {
            // We add newlines to the output
            if (rules[3] % 4 != 0)
            {
                status = kStatus_SSS_InvalidArgument;
                printf( "newline rules[3] must be multiple of 4:%d", status);
            }
            b64_len += (b64_len / rules[3]) * 2;
        }
        b64_len += 1; // terminating null
        if (*encoded_size < b64_len)
        {
            status = kStatus_SSS_Fail;
            printf( "Length of encoded buffer too small:%d",status);
        }
        // Initialize the return length to 0
        *encoded_size = 0;

        // Loop through the byte array by 3 then map to 4 base 64 encoded characters
        for (data_idx = 0; data_idx < data_size; data_idx += 3)
        {
            // Add \r\n every n bytes if specified
            if (rules[3] && data_idx > 0 && (b64_idx - offset) % rules[3] == 0)
            {
                // as soon as we do this, we introduce an offset
                encoded[b64_idx++] = '\r';
                encoded[b64_idx++] = '\n';
                offset += 2;
            }

            id = (data[data_idx] & 0xFC) >> 2;
            encoded[b64_idx++] = base64Char(id, rules);
            id = (data[data_idx] & 0x03) << 4;
            if (data_idx + 1 < data_size)
            {
                id |= (data[data_idx + 1] & 0xF0) >> 4;
                encoded[b64_idx++] = base64Char(id, rules);
                id = (data[data_idx + 1] & 0x0F) << 2;
                if (data_idx + 2 < data_size)
                {
                    id |= (data[data_idx + 2] & 0xC0) >> 6;
                    encoded[b64_idx++] = base64Char(id, rules);
                    id = data[data_idx + 2] & 0x3F;
                    encoded[b64_idx++] = base64Char(id, rules);
                }
                else
                {
                    encoded[b64_idx++] = base64Char(id, rules);
                    encoded[b64_idx++] = base64Char(B64_IS_EQUAL, rules);
                }
            }
            else
            {
                encoded[b64_idx++] = base64Char(id, rules);
                encoded[b64_idx++] = base64Char(B64_IS_EQUAL, rules);
                encoded[b64_idx++] = base64Char(B64_IS_EQUAL, rules);
            }
        }

        // Strip any trailing nulls
        while (b64_idx > 1 && encoded[b64_idx - 1] == 0)
        {
            b64_idx--;
        }

        // Null terminate end
        encoded[b64_idx++] = 0;

        // Set the final encoded length (excluding terminating null)
        *encoded_size = b64_idx - 1;
    }
    while (false);
    return status;
}