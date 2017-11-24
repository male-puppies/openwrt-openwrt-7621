/*
 * @@-COPYRIGHT-START-@@
 *
 * Copyright (c) 2014 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 *
 * @@-COPYRIGHT-END-@@
 */

#ifndef _ATH_BAND_STEERING_API__
#define _ATH_BAND_STEERING_API__
#define NETLINK_BAND_STEERING_EVENT 21
#define BSTEERING_INVALID_RSSI 0
#define IEEE80211_ADDR_LEN 6
/**
 * Metadata about a probe request received from a client that is useful
 * for making band steering decisions.
 */
struct bs_probe_req_ind {
    /* The MAC address of the client that sent the probe request.*/
    u_int8_t sender_addr[IEEE80211_ADDR_LEN];
    /*  The RSSI of the received probe request.*/
    u_int8_t rssi;
};

/**
 * Metadata about an authentication message that was sent with a failure
 * code due to the client being prohibited by the ACL.
 */
struct bs_auth_reject_ind {
    /* The MAC address of the client to which the authentication message 
        was sent with a failure code.*/
    u_int8_t client_addr[IEEE80211_ADDR_LEN];
    /* The RSSI of the received authentication message (the one that
       triggered the rejection).*/
    u_int8_t rssi;
};

/**
 * Metadata about a STA that has been authorized (security handshake has
 * completed).
 */
struct bs_node_authorized_ind {
    /* The MAC address of the client that is having its authorize status
       changed.*/
    u_int8_t client_addr[IEEE80211_ADDR_LEN];
};

/**
 * Metadata about a client activity status change.
 */
struct bs_activity_change_ind {
    /* The MAC address of the client that activity status changes */
    u_int8_t client_addr[IEEE80211_ADDR_LEN];
    /* Activity status*/
    u_int8_t activity;
};

/**
 * Data for a channel utilization measurement.
 */
struct bs_chan_utilization_ind {
    /* The current utilization on the band, expressed as a percentage.*/
    u_int8_t utilization;
};

/**
 * Enumeration to mark RSSI crossing direction
 */
typedef enum {
    /* RSSI not change */
    BSTEERING_RSSI_UNCHANGED = 0,
    /* RSSI going up */
    BSTEERING_RSSI_UP = 1,
    /* RSSI going down */
    BSTEERING_RSSI_DOWN = 2
} BSTEERING_RSSI_XING_DIRECTION;

/**
 * Metadata about a client RSSI measurement crossed threshold.
 */
struct bs_rssi_xing_threshold_ind {
    /* The MAC address of the client */
    u_int8_t client_addr[IEEE80211_ADDR_LEN];
    /* The measured RSSI */
    u_int8_t rssi;
    /* Flag indicating if it crossed inactivity RSSI threshold */
    BSTEERING_RSSI_XING_DIRECTION inact_rssi_xing;
    /* Flag indicating if it crossed low RSSI threshold */
    BSTEERING_RSSI_XING_DIRECTION low_rssi_xing;
};

/**
 * Metadata about a client requested RSSI measurement
 */
struct bs_rssi_measurement_ind {
    /* The MAC address of the client */
    u_int8_t client_addr[IEEE80211_ADDR_LEN];
    /* The measured RSSI */
    u_int8_t rssi;
};

/**
 * Common event structure for all Netlink indications to userspace.
 */
typedef struct ath_netlink_bsteering_event {
    /* The type of the event: One of ATH_BSTEERING_EVENT.*/
    u_int32_t type;
    /* The band on which the event occurred.*/
    u_int32_t band_index;
    /* The data for the event. Which member is valid is based on the
       type field.*/
    union {
        struct bs_probe_req_ind bs_probe;
        struct bs_node_authorized_ind bs_node_authorized;
        struct bs_activity_change_ind bs_activity_change;
        struct bs_auth_reject_ind bs_auth;
        struct bs_chan_utilization_ind bs_chan_util;
        struct bs_rssi_xing_threshold_ind bs_rssi_xing;
        struct bs_rssi_measurement_ind bs_rssi_measurement;
    } data;
} ath_netlink_bsteering_event_t;

/**
 * Parameters that can be configured by userspace to control the band
 * steering events.
 */
typedef struct ieee80211_bsteering_param_t {
    /* Amount of time a client has to be idle under normal (no overload)
       conditions before it becomes a candidate for steering.*/
    u_int32_t inactivity_timeout_normal;
    /*  Amount of time a client has to be idle under overload conditions
        before it becomes a candidate for steering.*/
    u_int32_t inactivity_timeout_overload;
    /* Frequency (in seconds) at which the client inactivity staus should
       be checked. */
    u_int32_t inactivity_check_period;
    /* Frequency (in seconds) at which the medium utilization should be
       measured. */
    u_int32_t utilization_sample_period;
    /* The number of samples over which the medium utilization should be
       averaged before being reported.*/
    u_int32_t utilization_average_num_samples;
    /* The RSSI value for which to generate threshold crossing events for
     an idle client. Such events are generated when the threshold is
     crossed in either direction.*/
    u_int32_t inactive_rssi_crossing_threshold;
    /* The RSSI value for which to generate threshold crossing events for
        both active and idle clients. This value should generally be less
        than inactive_rssi_crossing_threshold.*/
    u_int32_t low_rssi_crossing_threshold;
} ieee80211_bsteering_param_t;

/**
 * Parameters that are used to configure lmac part band steering logic.
 * Currently it contains inactivity related parameters.
 */
typedef struct ieee80211_bsteering_lmac_param_t {
    /* Frequency (in seconds) at which the client inactivity staus should
       be checked.*/
    u_int32_t inactivity_check_period;

    /* Number of check periods a client has to be idle under normal (no overload)
       conditions before it becomes a candidate for steering.*/
    u_int32_t inactivity_threshold_normal;

    /* Number of check periods a client has to be idle under overload conditions
       before it becomes a candidate for steering.*/
    u_int32_t inactivity_threshold_overload;
} ieee80211_bsteering_lmac_param_t;

/**
 * Parameters that must be specified to trigger an RSSI measurement by
 * sending QoS Null Data Packets and examining the RSSI from the ACK.
 */
typedef struct ieee80211_bsteering_rssi_req_t {
    /* The address of the client to measure.*/
    u_int8_t sender_addr[IEEE80211_ADDR_LEN];
    /* The number of consecutive measurements to make. This must be
       at least 1.*/
    u_int16_t num_measurements;
} ieee80211_bsteering_rssi_req_t;

/**
 * Parameters that can be configured by userspace to enable logging of
 * intermediate results via events to userspace.
 */
typedef struct ieee80211_bsteering_dbg_param_t {
    /* Whether logging of the raw channel utilization data is enabled.*/
    u_int8_t  raw_chan_util_log_enable:1;
    /* Whether logging of the raw RSSI measurement is enabled.*/
    u_int8_t  raw_rssi_log_enable:1;
} ieee80211_bsteering_dbg_param_t;

/**
 * Event types that are asynchronously generated by the band steering
 * module.
 */
typedef enum {
    /* Indication of utilization of the channel.*/
    ATH_EVENT_BSTEERING_CHAN_UTIL = 1,
    /* Indication that a probe request was received from a client.*/
    ATH_EVENT_BSTEERING_PROBE_REQ = 2,
    /* Authorization change for a STA.*/
    ATH_EVENT_BSTEERING_NODE_AUTHORIZED = 3,
    /* Indication that an authentication frame was sent with a failure
        status code.*/
    ATH_EVENT_BSTEERING_TX_AUTH_FAIL = 4,
    /* Indication that a client changes from active to inactive or
       vice versa.*/
    ATH_EVENT_BSTEERING_CLIENT_ACTIVITY_CHANGE = 5,
    /* Indication when the client RSSI crosses above or below the
       configured threshold.*/
    ATH_EVENT_BSTEERING_CLIENT_RSSI_CROSSING = 6,
    /* Indication when a requested RSSI measurement for a specific
       client is available.*/
    ATH_EVENT_BSTEERING_CLIENT_RSSI_MEASUREMENT = 7,
    /*  Events generated solely for debugging purposes. These are not
        intended for direct consumption by any algorithm components but are
        here to facilitate logging the raw data.*/
    ATH_EVENT_BSTEERING_DBG_CHAN_UTIL = 32,
    /* Raw RSSI measurement event used to facilitate logging.*/
    ATH_EVENT_BSTEERING_DBG_RSSI = 33,
} ATH_BSTEERING_EVENT;

/**
 * The band of operation.
 */
typedef enum {
    /* 2.4 GHz*/
    BSTEERING_24G = 0,
    /* 5 GHz */
    BSTEERING_5G = 1,
    BSTEERING_INVALID = 2,
    BSTEERING_MAX = BSTEERING_INVALID,
} BSTEERING_BAND;

#undef IEEE80211_ADDR_LEN
#endif /* _ATH_BAND_STEERING_API__ */
